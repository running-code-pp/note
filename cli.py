#!/usr/bin/env python3
"""
Command Line Interface for Note Management System

Provides a simple CLI for creating, reading, updating, and deleting notes.
"""

import argparse
import sys
from typing import List
from notes import NoteManager, Note


def format_note(note: Note, show_full: bool = False) -> str:
    """Format a note for display."""
    created = note.created_at[:19].replace('T', ' ')
    updated = note.updated_at[:19].replace('T', ' ')
    
    if show_full:
        return f"""
ID: {note.id}
Title: {note.title}
Created: {created}
Updated: {updated}
Content:
{note.content}
{'-' * 50}"""
    else:
        content_preview = note.content[:100] + "..." if len(note.content) > 100 else note.content
        return f"[{note.id[:8]}] {note.title} | {content_preview}"


def list_notes(manager: NoteManager, args) -> None:
    """List all notes."""
    notes = manager.get_all_notes()
    
    if not notes:
        print("No notes found.")
        return
    
    print(f"Found {len(notes)} note(s):")
    print()
    
    for note in notes:
        print(format_note(note, args.full))
        if args.full:
            print()


def create_note(manager: NoteManager, args) -> None:
    """Create a new note."""
    title = args.title
    content = args.content or ""
    
    if not title:
        print("Error: Note title is required.")
        sys.exit(1)
    
    # If content is not provided via argument, try to read from stdin
    if not content and not sys.stdin.isatty():
        content = sys.stdin.read().strip()
    
    try:
        note = manager.create_note(title, content)
        print(f"Created note: {note.id}")
        print(f"Title: {note.title}")
        if args.verbose:
            print(format_note(note, True))
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)


def find_note_by_id_prefix(manager: NoteManager, id_prefix: str) -> Note:
    """Find a note by ID or ID prefix."""
    # First try exact match
    note = manager.get_note(id_prefix)
    if note:
        return note
    
    # Then try prefix match
    matching_notes = []
    for note_id, note in manager.notes.items():
        if note_id.startswith(id_prefix):
            matching_notes.append(note)
    
    if len(matching_notes) == 1:
        return matching_notes[0]
    elif len(matching_notes) > 1:
        print(f"Ambiguous ID prefix '{id_prefix}'. Matching notes:")
        for note in matching_notes:
            print(f"  {note.id[:8]} - {note.title}")
        sys.exit(1)
    
    return None


def show_note(manager: NoteManager, args) -> None:
    """Show a specific note."""
    note = find_note_by_id_prefix(manager, args.id)
    
    if not note:
        print(f"Note with ID '{args.id}' not found.")
        sys.exit(1)
    
    print(format_note(note, True))


def update_note(manager: NoteManager, args) -> None:
    """Update an existing note."""
    note = find_note_by_id_prefix(manager, args.id)
    
    if not note:
        print(f"Note with ID '{args.id}' not found.")
        sys.exit(1)
    
    title = args.title
    content = args.content
    
    # If content is not provided via argument, try to read from stdin
    if content is None and not sys.stdin.isatty():
        content = sys.stdin.read().strip()
    
    if title is None and content is None:
        print("Error: Provide either --title or --content (or both) to update.")
        sys.exit(1)
    
    try:
        success = manager.update_note(note.id, title, content)
        if success:
            print(f"Updated note: {args.id}")
            if args.verbose:
                updated_note = manager.get_note(note.id)
                print(format_note(updated_note, True))
        else:
            print("Failed to update note.")
            sys.exit(1)
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)


def delete_note(manager: NoteManager, args) -> None:
    """Delete a note."""
    note = find_note_by_id_prefix(manager, args.id)
    
    if not note:
        print(f"Note with ID '{args.id}' not found.")
        sys.exit(1)
    
    if not args.force:
        print(f"Are you sure you want to delete note '{note.title}'? (y/N): ", end='')
        confirmation = input().lower()
        if confirmation not in ['y', 'yes']:
            print("Deletion cancelled.")
            return
    
    success = manager.delete_note(note.id)
    if success:
        print(f"Deleted note: {args.id}")
    else:
        print("Failed to delete note.")
        sys.exit(1)


def search_notes(manager: NoteManager, args) -> None:
    """Search notes."""
    if not args.query:
        print("Error: Search query is required.")
        sys.exit(1)
    
    notes = manager.search_notes(args.query)
    
    if not notes:
        print(f"No notes found matching '{args.query}'.")
        return
    
    print(f"Found {len(notes)} note(s) matching '{args.query}':")
    print()
    
    for note in notes:
        print(format_note(note, args.full))
        if args.full:
            print()


def main():
    """Main CLI entry point."""
    parser = argparse.ArgumentParser(
        description="Simple Note Management System",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s list                          # List all notes
  %(prog)s create "Meeting Notes" "..."  # Create a note
  %(prog)s show abc123                   # Show note by ID
  %(prog)s update abc123 --title "New"   # Update note title
  %(prog)s delete abc123                 # Delete a note
  %(prog)s search "keyword"              # Search notes
  echo "content" | %(prog)s create "Title"  # Create with piped content
        """
    )
    
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Show verbose output')
    parser.add_argument('--storage', '-s', default='notes.json',
                        help='Storage file path (default: notes.json)')
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List all notes')
    list_parser.add_argument('--full', '-f', action='store_true',
                             help='Show full note content')
    
    # Create command
    create_parser = subparsers.add_parser('create', help='Create a new note')
    create_parser.add_argument('title', help='Note title')
    create_parser.add_argument('content', nargs='?', default='',
                               help='Note content (or pipe via stdin)')
    
    # Show command
    show_parser = subparsers.add_parser('show', help='Show a specific note')
    show_parser.add_argument('id', help='Note ID (or unique prefix)')
    
    # Update command
    update_parser = subparsers.add_parser('update', help='Update an existing note')
    update_parser.add_argument('id', help='Note ID (or unique prefix)')
    update_parser.add_argument('--title', '-t', help='New title')
    update_parser.add_argument('--content', '-c', help='New content (or pipe via stdin)')
    
    # Delete command
    delete_parser = subparsers.add_parser('delete', help='Delete a note')
    delete_parser.add_argument('id', help='Note ID (or unique prefix)')
    delete_parser.add_argument('--force', '-f', action='store_true',
                               help='Delete without confirmation')
    
    # Search command
    search_parser = subparsers.add_parser('search', help='Search notes')
    search_parser.add_argument('query', help='Search query')
    search_parser.add_argument('--full', '-f', action='store_true',
                               help='Show full note content')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    # Initialize the note manager
    manager = NoteManager(args.storage)
    
    # Dispatch to appropriate handler
    handlers = {
        'list': list_notes,
        'create': create_note,
        'show': show_note,
        'update': update_note,
        'delete': delete_note,
        'search': search_notes,
    }
    
    handler = handlers.get(args.command)
    if handler:
        handler(manager, args)
    else:
        print(f"Unknown command: {args.command}")
        sys.exit(1)


if __name__ == "__main__":
    main()