#!/usr/bin/env python3
"""
Simple Note Management System

A lightweight note-taking application that allows users to create, read, update, 
and delete notes with persistence to local storage.
"""

import json
import os
import uuid
from datetime import datetime
from typing import List, Optional, Dict, Any


class Note:
    """Represents a single note with metadata."""
    
    def __init__(self, title: str, content: str, note_id: str = None):
        self.id = note_id or str(uuid.uuid4())
        self.title = title
        self.content = content
        self.created_at = datetime.now().isoformat()
        self.updated_at = self.created_at
    
    def update(self, title: str = None, content: str = None) -> None:
        """Update the note's title and/or content."""
        if title is not None:
            self.title = title
        if content is not None:
            self.content = content
        self.updated_at = datetime.now().isoformat()
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert note to dictionary for serialization."""
        return {
            'id': self.id,
            'title': self.title,
            'content': self.content,
            'created_at': self.created_at,
            'updated_at': self.updated_at
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'Note':
        """Create a Note instance from dictionary data."""
        note = cls(data['title'], data['content'], data['id'])
        note.created_at = data['created_at']
        note.updated_at = data['updated_at']
        return note
    
    def __str__(self) -> str:
        return f"Note(id={self.id[:8]}..., title='{self.title}')"
    
    def __repr__(self) -> str:
        return self.__str__()


class NoteManager:
    """Manages a collection of notes with persistence."""
    
    def __init__(self, storage_file: str = "notes.json"):
        self.storage_file = storage_file
        self.notes: Dict[str, Note] = {}
        self.load_notes()
    
    def load_notes(self) -> None:
        """Load notes from storage file."""
        if os.path.exists(self.storage_file):
            try:
                with open(self.storage_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    self.notes = {
                        note_id: Note.from_dict(note_data)
                        for note_id, note_data in data.items()
                    }
            except (json.JSONDecodeError, KeyError) as e:
                print(f"Error loading notes: {e}")
                self.notes = {}
    
    def save_notes(self) -> None:
        """Save notes to storage file."""
        try:
            data = {note_id: note.to_dict() for note_id, note in self.notes.items()}
            with open(self.storage_file, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
        except IOError as e:
            print(f"Error saving notes: {e}")
    
    def create_note(self, title: str, content: str) -> Note:
        """Create a new note."""
        if not title.strip():
            raise ValueError("Note title cannot be empty")
        
        note = Note(title.strip(), content.strip())
        self.notes[note.id] = note
        self.save_notes()
        return note
    
    def get_note(self, note_id: str) -> Optional[Note]:
        """Retrieve a note by ID."""
        return self.notes.get(note_id)
    
    def get_all_notes(self) -> List[Note]:
        """Get all notes sorted by creation date."""
        return sorted(self.notes.values(), key=lambda n: n.created_at, reverse=True)
    
    def update_note(self, note_id: str, title: str = None, content: str = None) -> bool:
        """Update an existing note."""
        note = self.notes.get(note_id)
        if not note:
            return False
        
        if title is not None and not title.strip():
            raise ValueError("Note title cannot be empty")
        
        note.update(
            title.strip() if title is not None else None,
            content.strip() if content is not None else None
        )
        self.save_notes()
        return True
    
    def delete_note(self, note_id: str) -> bool:
        """Delete a note by ID."""
        if note_id in self.notes:
            del self.notes[note_id]
            self.save_notes()
            return True
        return False
    
    def search_notes(self, query: str) -> List[Note]:
        """Search notes by title or content."""
        query_lower = query.lower()
        matching_notes = []
        
        for note in self.notes.values():
            if (query_lower in note.title.lower() or 
                query_lower in note.content.lower()):
                matching_notes.append(note)
        
        return sorted(matching_notes, key=lambda n: n.updated_at, reverse=True)
    
    def get_note_count(self) -> int:
        """Get the total number of notes."""
        return len(self.notes)


if __name__ == "__main__":
    # Simple demonstration
    manager = NoteManager()
    
    # Create a sample note
    note = manager.create_note("Welcome", "This is your first note!")
    print(f"Created note: {note}")
    
    # List all notes
    all_notes = manager.get_all_notes()
    print(f"Total notes: {len(all_notes)}")
    
    for note in all_notes:
        print(f"- {note.title}: {note.content[:50]}...")