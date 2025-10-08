# Note Management System

A simple, lightweight command-line note-taking application written in Python. Create, read, update, delete, and search your notes with ease.

## Features

- **Create Notes**: Add new notes with title and content
- **List Notes**: View all your notes with previews
- **Show Notes**: Display full note content with metadata
- **Update Notes**: Edit existing note titles and content
- **Delete Notes**: Remove notes you no longer need
- **Search Notes**: Find notes by title or content keywords
- **Persistent Storage**: Notes are saved to a JSON file automatically
- **Partial ID Matching**: Use short ID prefixes for quick note operations

## Installation

No external dependencies required! This application uses only Python standard library modules.

```bash
git clone <repository-url>
cd note
```

## Quick Start

### Create your first note:
```bash
python cli.py create "My First Note" "This is the content of my first note."
```

### List all notes:
```bash
python cli.py list
```

### Show a specific note:
```bash
python cli.py show abc123  # Use the ID or ID prefix shown in list
```

## Usage

### Basic Commands

**Create a note:**
```bash
python cli.py create "Meeting Notes" "Discussed project timeline and deliverables."
```

**Create a note with piped content:**
```bash
echo "Important reminder for tomorrow" | python cli.py create "Reminder"
```

**List all notes:**
```bash
python cli.py list
```

**List notes with full content:**
```bash
python cli.py list --full
```

**Show a specific note:**
```bash
python cli.py show abc123
```

**Update a note's title:**
```bash
python cli.py update abc123 --title "Updated Title"
```

**Update a note's content:**
```bash
python cli.py update abc123 --content "New content here"
```

**Update both title and content:**
```bash
python cli.py update abc123 --title "New Title" --content "New content"
```

**Search notes:**
```bash
python cli.py search "keyword"
```

**Delete a note:**
```bash
python cli.py delete abc123
```

**Delete without confirmation:**
```bash
python cli.py delete abc123 --force
```

### Advanced Usage

**Custom storage file:**
```bash
python cli.py --storage /path/to/my-notes.json list
```

**Verbose output:**
```bash
python cli.py --verbose create "Note Title" "Content"
```

## API Usage

You can also use the note system programmatically:

```python
from notes import NoteManager

# Initialize manager
manager = NoteManager("my-notes.json")

# Create a note
note = manager.create_note("API Example", "Created via Python API")

# Get all notes
all_notes = manager.get_all_notes()

# Search notes
results = manager.search_notes("example")

# Update a note
manager.update_note(note.id, title="Updated Title")

# Delete a note
manager.delete_note(note.id)
```

## File Structure

```
note/
├── notes.py           # Core note management classes
├── cli.py             # Command-line interface
├── test_notes.py      # Test suite
├── requirements.txt   # Dependencies (none required)
└── README.md         # This file
```

## Storage Format

Notes are stored in JSON format with the following structure:

```json
{
  "note-id-1": {
    "id": "note-id-1",
    "title": "Note Title",
    "content": "Note content...",
    "created_at": "2023-10-08T16:41:39.123456",
    "updated_at": "2023-10-08T16:41:39.123456"
  }
}
```

## Testing

Run the test suite:

```bash
python -m unittest test_notes.py -v
```

## License

This project is released under the MIT License.