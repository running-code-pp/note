#include <asio.hpp>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <functional>
#include <iostream>
#include <chrono>

// 前向声明
class Connection; 

// 连接池配置结构
struct ConnectionPoolConfig {
    std::string host;              // 服务器主机名
    std::string port;              // 服务器端口
    size_t min_connections = 2;    // 最小连接数
    size_t max_connections = 10;   // 最大连接数
    std::chrono::seconds connection_timeout = std::chrono::seconds(5);  // 连接超时时间
    std::chrono::seconds idle_timeout = std::chrono::seconds(60);       // 空闲连接超时
    std::chrono::seconds health_check_interval = std::chrono::seconds(30); // 健康检查间隔
};

// 连接状态枚举
enum class ConnectionStatus {
    DISCONNECTED,  // 未连接
    CONNECTING,    // 连接中
    CONNECTED,     // 已连接
    CLOSING,       // 关闭中
    HEALTH_CHECKING // 健康检查中
};

// 连接类
class Connection : public std::enable_shared_from_this<Connection> {
public:
    using Ptr = std::shared_ptr<Connection>;
    using ErrorCallback = std::function<void(const asio::error_code&, Connection::Ptr)>;
    using ConnectCallback = std::function<void(bool, Connection::Ptr)>;
    using HealthCheckCallback = std::function<void(bool)>;

    Connection(asio::io_context& io_context, const std::string& host, const std::string& port)
        : socket_(io_context),
          resolver_(io_context),
          host_(host),
          port_(port),
          status_(ConnectionStatus::DISCONNECTED),
          last_activity_(std::chrono::steady_clock::now()),
          reconnect_attempts_(0),
          max_reconnect_attempts_(3) {
    }

    ~Connection() {
        close();
    }

    // 连接到服务器
    void connect(ConnectCallback callback, const std::chrono::seconds& timeout = std::chrono::seconds(5)) {
        if (status_ != ConnectionStatus::DISCONNECTED) {
            callback(false, shared_from_this());
            return;
        }

        status_ = ConnectionStatus::CONNECTING;
        reconnect_attempts_ = 0;

        // 设置连接超时定时器
        timeout_timer_ = std::make_unique<asio::steady_timer>(socket_.get_executor(), timeout);
        timeout_timer_->async_wait([this, self = shared_from_this()](const asio::error_code& ec) {
            if (!ec && status_ == ConnectionStatus::CONNECTING) {
                // 连接超时
                std::cerr << "Connection timeout to " << host_ << ":" << port_ << std::endl;
                status_ = ConnectionStatus::DISCONNECTED;
                if (connect_callback_) {
                    connect_callback_(false, self);
                }
            }
        });

        connect_callback_ = std::move(callback);

        resolver_.async_resolve(host_, port_, [this, self = shared_from_this()](
            const asio::error_code& ec, asio::ip::tcp::resolver::results_type results) {
            if (ec) {
                handle_connect_error(ec);
                return;
            }

            asio::async_connect(socket_, results, [this, self = shared_from_this()](
                const asio::error_code& ec, const asio::ip::tcp::endpoint& endpoint) {
                // 取消超时定时器
                if (timeout_timer_) {
                    timeout_timer_->cancel();
                }

                if (ec) {
                    handle_connect_error(ec);
                    return;
                }

                // 连接成功
                status_ = ConnectionStatus::CONNECTED;
                last_activity_ = std::chrono::steady_clock::now();
                std::cout << "Connected to " << endpoint << std::endl;
                
                if (connect_callback_) {
                    connect_callback_(true, self);
                }
            });
        });
    }

    // 异步写入数据
    template<typename ConstBufferSequence, typename WriteHandler>
    void async_write(const ConstBufferSequence& buffers, WriteHandler handler) {
        if (status_ != ConnectionStatus::CONNECTED) {
            asio::post(socket_.get_executor(), [handler]() {
                handler(asio::error_code(asio::error::not_connected), 0);
            });
            return;
        }

        last_activity_ = std::chrono::steady_clock::now();
        asio::async_write(socket_, buffers, [this, handler = std::move(handler)](
            const asio::error_code& ec, size_t bytes_transferred) {
            if (ec) {
                handle_io_error(ec);
            }
            handler(ec, bytes_transferred);
        });
    }

    // 异步读取数据
    template<typename MutableBufferSequence, typename ReadHandler>
    void async_read_some(const MutableBufferSequence& buffers, ReadHandler handler) {
        if (status_ != ConnectionStatus::CONNECTED) {
            asio::post(socket_.get_executor(), [handler]() {
                handler(asio::error_code(asio::error::not_connected), 0);
            });
            return;
        }

        last_activity_ = std::chrono::steady_clock::now();
        socket_.async_read_some(buffers, [this, handler = std::move(handler)](
            const asio::error_code& ec, size_t bytes_transferred) {
            if (ec) {
                handle_io_error(ec);
            }
            handler(ec, bytes_transferred);
        });
    }

    // 关闭连接
    void close() {
        if (status_ == ConnectionStatus::DISCONNECTED) {
            return;
        }

        status_ = ConnectionStatus::CLOSING;
        
        asio::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        if (ec && ec != asio::error::not_connected) {
            std::cerr << "Error shutting down socket: " << ec.message() << std::endl;
        }

        socket_.close(ec);
        if (ec) {
            std::cerr << "Error closing socket: " << ec.message() << std::endl;
        }

        status_ = ConnectionStatus::DISCONNECTED;
    }

    // 检查连接是否打开
    bool is_open() const {
        return socket_.is_open() && status_ == ConnectionStatus::CONNECTED;
    }

    // 获取连接状态
    ConnectionStatus get_status() const {
        return status_;
    }

    // 执行健康检查
    void perform_health_check(HealthCheckCallback callback) {
        if (status_ != ConnectionStatus::CONNECTED) {
            callback(false);
            return;
        }

        status_ = ConnectionStatus::HEALTH_CHECKING;

        // 在实际应用中，这里应该发送一个简单的健康检查请求
        // 这里只是模拟一个快速的检查
        asio::post(socket_.get_executor(), [this, self = shared_from_this(), callback = std::move(callback)]() {
            // 假设检查总是成功的
            // 在真实场景中，应该发送一个ping或其他简单请求
            status_ = ConnectionStatus::CONNECTED;
            callback(true);
        });
    }

    // 设置错误处理回调
    void set_error_callback(ErrorCallback callback) {
        error_callback_ = std::move(callback);
    }

    // 获取最后活动时间
    std::chrono::steady_clock::time_point get_last_activity_time() const {
        return last_activity_;
    }

private:
    // 处理连接错误
    void handle_connect_error(const asio::error_code& ec) {
        std::cerr << "Connection error: " << ec.message() << std::endl;
        status_ = ConnectionStatus::DISCONNECTED;
        
        if (reconnect_attempts_ < max_reconnect_attempts_) {
            // 尝试重连
            reconnect_attempts_++;
            std::cout << "Attempting to reconnect (" << reconnect_attempts_ << "/" << max_reconnect_attempts_ << ")..." << std::endl;
            
            // 指数退避策略
            auto delay = std::chrono::milliseconds(100 * (1 << reconnect_attempts_));
            asio::post(socket_.get_executor(), [this, self = shared_from_this(), delay]() {
                asio::steady_timer timer(socket_.get_executor(), delay);
                timer.async_wait([this, self](const asio::error_code&) {
                    if (status_ == ConnectionStatus::DISCONNECTED && connect_callback_) {
                        connect(connect_callback_);
                    }
                });
            });
        } else {
            // 重连失败
            if (connect_callback_) {
                connect_callback_(false, shared_from_this());
            }
        }
    }

    // 处理IO错误
    void handle_io_error(const asio::error_code& ec) {
        std::cerr << "IO error: " << ec.message() << std::endl;
        
        // 连接已关闭或重置
        if (ec == asio::error::eof || ec == asio::error::connection_reset) {
            close();
            if (error_callback_) {
                error_callback_(ec, shared_from_this());
            }
        }
    }

    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
    std::unique_ptr<asio::steady_timer> timeout_timer_;
    
    std::string host_;
    std::string port_;
    ConnectionStatus status_;
    
    std::chrono::steady_clock::time_point last_activity_;
    int reconnect_attempts_;
    const int max_reconnect_attempts_;
    
    ConnectCallback connect_callback_;
    ErrorCallback error_callback_;
};

// 连接池类
class ConnectionPool : public std::enable_shared_from_this<ConnectionPool> {
public:
    using Ptr = std::shared_ptr<ConnectionPool>;
    using ConnectionHandler = std::function<void(Connection::Ptr)>;

    ConnectionPool(asio::io_context& io_context, const ConnectionPoolConfig& config)
        : io_context_(io_context),
          config_(config),
          strand_(io_context),
          total_connections_(0),
          is_running_(false) {
    }

    ~ConnectionPool() {
        stop();
    }

    // 启动连接池
    void start() {
        asio::post(strand_, [this, self = shared_from_this()]() {
            if (is_running_) {
                return;
            }

            is_running_ = true;

            // 创建最小数量的连接
            for (size_t i = 0; i < config_.min_connections; ++i) {
                create_connection();
            }

            // 启动健康检查定时器
            start_health_check();
        });
    }

    // 停止连接池
    void stop() {
        asio::post(strand_, [this]() {
            if (!is_running_) {
                return;
            }

            is_running_ = false;
            
            // 取消健康检查定时器
            health_check_timer_.cancel();
            
            // 清空等待队列
            waiting_handlers_.clear();
            
            // 关闭所有连接
            for (auto& conn : available_connections_) {
                conn->close();
            }
            for (auto& conn : in_use_connections_) {
                conn->close();
            }
            
            available_connections_.clear();
            in_use_connections_.clear();
            total_connections_ = 0;
        });
    }

    // 从连接池获取一个连接
    void get_connection(ConnectionHandler handler) {
        asio::post(strand_, [this, self = shared_from_this(), handler = std::move(handler)]() {
            // 检查是否有可用连接
            if (!available_connections_.empty()) {
                auto connection = std::move(available_connections_.front());
                available_connections_.pop_front();
                
                // 将连接标记为正在使用
                in_use_connections_.push_back(connection);
                
                // 提交到io_context，确保在正确的线程中执行
                asio::post(io_context_, std::bind(handler, connection));
                return;
            }

            // 如果没有达到最大连接数，创建新连接
            if (total_connections_ < config_.max_connections) {
                create_connection_for_handler(std::move(handler));
                return;
            }

            // 否则，加入等待队列
            waiting_handlers_.push_back(std::move(handler));
        });
    }

    // 归还连接到连接池
    void return_connection(Connection::Ptr connection) {
        asio::post(strand_, [this, connection = std::move(connection)]() {
            // 从正在使用的连接列表中移除
            auto it = std::find(in_use_connections_.begin(), in_use_connections_.end(), connection);
            if (it != in_use_connections_.end()) {
                in_use_connections_.erase(it);
            }

            // 检查连接是否仍然有效
            if (!connection->is_open()) {
                // 连接已关闭，减少总连接数
                total_connections_--;
                
                // 如果需要，创建新连接以维持最小连接数
                if (total_connections_ < config_.min_connections) {
                    create_connection();
                }
                return;
            }

            // 检查是否有等待的处理程序
            if (!waiting_handlers_.empty()) {
                auto handler = std::move(waiting_handlers_.front());
                waiting_handlers_.pop_front();
                
                // 重新将连接标记为正在使用
                in_use_connections_.push_back(connection);
                
                // 提交到io_context
                asio::post(io_context_, std::bind(handler, connection));
            } else {
                // 否则，将连接放回可用连接池
                available_connections_.push_back(connection);
            }
        });
    }

    // 获取连接池状态信息
    struct Status {
        size_t total_connections;
        size_t available_connections;
        size_t in_use_connections;
        size_t waiting_handlers;
    };

    Status get_status() {
        std::lock_guard<std::mutex> lock(status_mutex_);
        return status_;
    }

private:
    // 创建一个新连接
    void create_connection() {
        create_connection_for_handler(nullptr);
    }

    // 创建一个新连接并分配给处理程序（如果有）
    void create_connection_for_handler(ConnectionHandler handler) {
        total_connections_++;
        
        auto connection = std::make_shared<Connection>(io_context_, config_.host, config_.port);
        
        // 设置错误处理回调
        connection->set_error_callback([this, self = shared_from_this()](const asio::error_code& ec, Connection::Ptr conn) {
            handle_connection_error(ec, conn);
        });

        connection->connect([this, connection, handler](bool success, Connection::Ptr conn) {
            asio::post(strand_, [this, success, connection, handler]() {
                if (!success) {
                    // 连接失败
                    total_connections_--;
                    
                    // 如果有处理程序等待，尝试创建另一个连接
                    if (handler) {
                        create_connection_for_handler(std::move(handler));
                    }
                    return;
                }

                // 更新状态信息
                update_status();

                if (handler) {
                    // 有处理程序等待，直接分配连接
                    in_use_connections_.push_back(connection);
                    asio::post(io_context_, std::bind(handler, connection));
                } else {
                    // 没有处理程序等待，加入可用连接池
                    available_connections_.push_back(connection);
                }
            });
        }, config_.connection_timeout);
    }

    // 处理连接错误
    void handle_connection_error(const asio::error_code& ec, Connection::Ptr connection) {
        asio::post(strand_, [this, connection]() {
            // 从可用连接或正在使用的连接列表中移除
            auto it_available = std::find(available_connections_.begin(), available_connections_.end(), connection);
            if (it_available != available_connections_.end()) {
                available_connections_.erase(it_available);
            }

            auto it_in_use = std::find(in_use_connections_.begin(), in_use_connections_.end(), connection);
            if (it_in_use != in_use_connections_.end()) {
                in_use_connections_.erase(it_in_use);
            }

            // 减少总连接数
            total_connections_--;

            // 更新状态信息
            update_status();

            // 创建新连接以维持最小连接数
            if (is_running_ && total_connections_ < config_.min_connections) {
                create_connection();
            }
        });
    }

    // 启动健康检查
    void start_health_check() {
        if (!is_running_) {
            return;
        }

        health_check_timer_ = asio::steady_timer(io_context_, config_.health_check_interval);
        health_check_timer_.async_wait([this, self = shared_from_this()](const asio::error_code& ec) {
            if (ec || !is_running_) {
                return;
            }

            perform_health_check();
            start_health_check(); // 重新安排下一次检查
        });
    }

    // 执行健康检查
    void perform_health_check() {
        auto now = std::chrono::steady_clock::now();
        std::vector<Connection::Ptr> connections_to_check;
        std::vector<Connection::Ptr> idle_connections_to_close;

        // 复制可用连接列表用于检查
        {   
            // 注意：这里不需要额外的锁，因为我们在strand中执行
            connections_to_check = available_connections_;
        }

        // 检查每个连接
        for (auto& connection : connections_to_check) {
            // 检查空闲超时
            auto idle_time = std::chrono::duration_cast<std::chrono::seconds>(
                now - connection->get_last_activity_time());
            
            if (idle_time > config_.idle_timeout && total_connections_ > config_.min_connections) {
                // 连接空闲时间过长且超过最小连接数，准备关闭
                idle_connections_to_close.push_back(connection);
            } else {
                // 执行健康检查
                connection->perform_health_check([this, connection](bool is_healthy) {
                    if (!is_healthy) {
                        // 连接不健康，关闭并重连
                        connection->close();
                        handle_connection_error(asio::error_code(), connection);
                    }
                });
            }
        }

        // 关闭超时的空闲连接
        for (auto& connection : idle_connections_to_close) {
            auto it = std::find(available_connections_.begin(), available_connections_.end(), connection);
            if (it != available_connections_.end()) {
                available_connections_.erase(it);
                connection->close();
                total_connections_--;
                update_status();
            }
        }
    }

    // 更新状态信息
    void update_status() {
        std::lock_guard<std::mutex> lock(status_mutex_);
        status_.total_connections = total_connections_;
        status_.available_connections = available_connections_.size();
        status_.in_use_connections = in_use_connections_.size();
        status_.waiting_handlers = waiting_handlers_.size();
    }

    asio::io_context& io_context_;
    ConnectionPoolConfig config_;
    asio::io_context::strand strand_; // 保护共享数据

    size_t total_connections_;
    std::deque<Connection::Ptr> available_connections_;
    std::vector<Connection::Ptr> in_use_connections_;
    std::queue<ConnectionHandler> waiting_handlers_;

    bool is_running_;
    asio::steady_timer health_check_timer_;

    mutable std::mutex status_mutex_;
    Status status_;
};

// 使用连接池的示例
class Client {
public:
    Client(asio::io_context& io_context) : io_context_(io_context) {
        // 配置连接池
        ConnectionPoolConfig config;
        config.host = "localhost";
        config.port = "8080";
        config.min_connections = 2;
        config.max_connections = 10;
        
        // 创建连接池
        connection_pool_ = std::make_shared<ConnectionPool>(io_context, config);
        
        // 启动连接池
        connection_pool_->start();
    }
    
    // 发送请求
    void send_request(const std::string& request_data, 
                      std::function<void(bool, const std::string&)> callback) {
        // 从连接池获取连接
        connection_pool_->get_connection([this, request_data, callback](Connection::Ptr connection) {
            if (!connection || !connection->is_open()) {
                callback(false, "Failed to get connection");
                return;
            }
            
            // 发送请求数据
            connection->async_write(asio::buffer(request_data),
                [this, connection, callback](const asio::error_code& ec, size_t) {
                    if (ec) {
                        // 处理写入错误
                        std::cerr << "Write error: " << ec.message() << std::endl;
                        connection_pool_->return_connection(connection);
                        callback(false, "Write failed: " + ec.message());
                        return;
                    }
                    
                    // 准备读取响应
                    std::make_shared<std::vector<char>>(1024)->resize(1024);
                    auto buffer = std::make_shared<std::vector<char>>(1024);
                    
                    connection->async_read_some(asio::buffer(*buffer),
                        [this, connection, buffer, callback](const asio::error_code& ec, size_t bytes_read) {
                            // 归还连接到连接池
                            connection_pool_->return_connection(connection);
                            
                            if (ec) {
                                std::cerr << "Read error: " << ec.message() << std::endl;
                                callback(false, "Read failed: " + ec.message());
                                return;
                            }
                            
                            // 处理响应数据
                            std::string response(buffer->data(), bytes_read);
                            callback(true, response);
                        });
                });
        });
    }
    
    // 关闭客户端
    void shutdown() {
        if (connection_pool_) {
            connection_pool_->stop();
        }
    }
    
private:
    asio::io_context& io_context_;
    ConnectionPool::Ptr connection_pool_;
};
