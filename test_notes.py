#!/usr/bin/env python3
"""
Tests for the Note Management System

Basic test suite to validate core functionality.
"""

import unittest
import tempfile
import os
from notes import Note, NoteManager


class TestNote(unittest.TestCase):
    """Test the Note class."""
    
    def test_note_creation(self):
        """Test creating a new note."""
        note = Note("Test Title", "Test Content")
        
        self.assertIsNotNone(note.id)
        self.assertEqual(note.title, "Test Title")
        self.assertEqual(note.content, "Test Content")
        self.assertIsNotNone(note.created_at)
        self.assertEqual(note.created_at, note.updated_at)
    
    def test_note_update(self):
        """Test updating a note."""
        note = Note("Original Title", "Original Content")
        original_created = note.created_at
        original_updated = note.updated_at
        
        # Small delay to ensure timestamp difference
        import time
        time.sleep(0.01)
        
        note.update("Updated Title", "Updated Content")
        
        self.assertEqual(note.title, "Updated Title")
        self.assertEqual(note.content, "Updated Content")
        self.assertEqual(note.created_at, original_created)  # Should not change
        self.assertNotEqual(note.updated_at, original_updated)  # Should change
    
    def test_note_partial_update(self):
        """Test partial update of a note."""
        note = Note("Original Title", "Original Content")
        
        # Update only title
        note.update(title="New Title")
        self.assertEqual(note.title, "New Title")
        self.assertEqual(note.content, "Original Content")
        
        # Update only content
        note.update(content="New Content")
        self.assertEqual(note.title, "New Title")
        self.assertEqual(note.content, "New Content")
    
    def test_note_serialization(self):
        """Test note to/from dictionary conversion."""
        original = Note("Test Title", "Test Content")
        note_dict = original.to_dict()
        
        # Verify dictionary structure
        expected_keys = {'id', 'title', 'content', 'created_at', 'updated_at'}
        self.assertEqual(set(note_dict.keys()), expected_keys)
        
        # Test round-trip conversion
        restored = Note.from_dict(note_dict)
        self.assertEqual(restored.id, original.id)
        self.assertEqual(restored.title, original.title)
        self.assertEqual(restored.content, original.content)
        self.assertEqual(restored.created_at, original.created_at)
        self.assertEqual(restored.updated_at, original.updated_at)


class TestNoteManager(unittest.TestCase):
    """Test the NoteManager class."""
    
    def setUp(self):
        """Set up test environment."""
        # Use temporary file for testing
        self.temp_file = tempfile.NamedTemporaryFile(mode='w', delete=False)
        self.temp_file.close()
        self.manager = NoteManager(self.temp_file.name)
    
    def tearDown(self):
        """Clean up test environment."""
        try:
            os.unlink(self.temp_file.name)
        except FileNotFoundError:
            pass
    
    def test_create_note(self):
        """Test creating a note."""
        note = self.manager.create_note("Test Title", "Test Content")
        
        self.assertIsNotNone(note.id)
        self.assertEqual(note.title, "Test Title")
        self.assertEqual(note.content, "Test Content")
        self.assertEqual(self.manager.get_note_count(), 1)
    
    def test_create_note_validation(self):
        """Test note creation validation."""
        # Empty title should raise ValueError
        with self.assertRaises(ValueError):
            self.manager.create_note("", "Content")
        
        with self.assertRaises(ValueError):
            self.manager.create_note("   ", "Content")
    
    def test_get_note(self):
        """Test retrieving a note."""
        created_note = self.manager.create_note("Test Title", "Test Content")
        retrieved_note = self.manager.get_note(created_note.id)
        
        self.assertIsNotNone(retrieved_note)
        self.assertEqual(retrieved_note.id, created_note.id)
        self.assertEqual(retrieved_note.title, created_note.title)
        
        # Test non-existent note
        self.assertIsNone(self.manager.get_note("nonexistent"))
    
    def test_get_all_notes(self):
        """Test retrieving all notes."""
        # Initially empty
        self.assertEqual(len(self.manager.get_all_notes()), 0)
        
        # Add some notes
        note1 = self.manager.create_note("Title 1", "Content 1")
        note2 = self.manager.create_note("Title 2", "Content 2")
        
        all_notes = self.manager.get_all_notes()
        self.assertEqual(len(all_notes), 2)
        
        # Should be sorted by creation date (newest first)
        self.assertEqual(all_notes[0].id, note2.id)
        self.assertEqual(all_notes[1].id, note1.id)
    
    def test_update_note(self):
        """Test updating a note."""
        note = self.manager.create_note("Original Title", "Original Content")
        
        # Update both title and content
        success = self.manager.update_note(note.id, "Updated Title", "Updated Content")
        self.assertTrue(success)
        
        updated_note = self.manager.get_note(note.id)
        self.assertEqual(updated_note.title, "Updated Title")
        self.assertEqual(updated_note.content, "Updated Content")
        
        # Update only title
        success = self.manager.update_note(note.id, title="New Title")
        self.assertTrue(success)
        
        updated_note = self.manager.get_note(note.id)
        self.assertEqual(updated_note.title, "New Title")
        self.assertEqual(updated_note.content, "Updated Content")
        
        # Test updating non-existent note
        success = self.manager.update_note("nonexistent", "Title")
        self.assertFalse(success)
        
        # Test empty title validation
        with self.assertRaises(ValueError):
            self.manager.update_note(note.id, title="")
    
    def test_delete_note(self):
        """Test deleting a note."""
        note = self.manager.create_note("Test Title", "Test Content")
        self.assertEqual(self.manager.get_note_count(), 1)
        
        # Delete the note
        success = self.manager.delete_note(note.id)
        self.assertTrue(success)
        self.assertEqual(self.manager.get_note_count(), 0)
        self.assertIsNone(self.manager.get_note(note.id))
        
        # Try to delete non-existent note
        success = self.manager.delete_note("nonexistent")
        self.assertFalse(success)
    
    def test_search_notes(self):
        """Test searching notes."""
        self.manager.create_note("Meeting Notes", "Discussed project timeline")
        self.manager.create_note("Shopping List", "Buy groceries and supplies")
        self.manager.create_note("Project Ideas", "New features for the application")
        
        # Search by title
        results = self.manager.search_notes("meeting")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, "Meeting Notes")
        
        # Search by content
        results = self.manager.search_notes("project")
        self.assertEqual(len(results), 2)  # Should find both notes containing "project"
        
        # Case-insensitive search
        results = self.manager.search_notes("SHOPPING")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].title, "Shopping List")
        
        # No matches
        results = self.manager.search_notes("nonexistent")
        self.assertEqual(len(results), 0)
    
    def test_persistence(self):
        """Test note persistence across manager instances."""
        # Create notes with first manager
        note1 = self.manager.create_note("Title 1", "Content 1")
        note2 = self.manager.create_note("Title 2", "Content 2")
        
        # Create new manager with same file
        new_manager = NoteManager(self.temp_file.name)
        
        # Should load existing notes
        self.assertEqual(new_manager.get_note_count(), 2)
        
        loaded_note1 = new_manager.get_note(note1.id)
        loaded_note2 = new_manager.get_note(note2.id)
        
        self.assertIsNotNone(loaded_note1)
        self.assertIsNotNone(loaded_note2)
        self.assertEqual(loaded_note1.title, "Title 1")
        self.assertEqual(loaded_note2.title, "Title 2")


if __name__ == "__main__":
    unittest.main()