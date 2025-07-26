"""
Tests for Question Analyzer Module
"""

import pytest
from core.question_analyzer import QuestionAnalyzer


class TestQuestionAnalyzer:
    """Test cases for QuestionAnalyzer."""
    
    def setup_method(self):
        """Set up test fixtures."""
        self.analyzer = QuestionAnalyzer()
    
    def test_analyze_calculation_question(self):
        """Test analysis of calculation questions."""
        question = "What is the sum of all prime numbers between 1 and 100?"
        result = self.analyzer.analyze(question)
        
        assert result['question_type'] == 'calculation'
        assert 'calculator' in result['required_tools']
    
    def test_analyze_research_question(self):
        """Test analysis of research questions."""
        question = "Who won the Nobel Prize in Physics in 2022?"
        result = self.analyzer.analyze(question)
        
        assert result['question_type'] == 'research'
        assert 'web_search' in result['required_tools']
    
    def test_analyze_with_attachments(self):
        """Test analysis with file attachments."""
        question = "What is the average value in the attached CSV file?"
        attachments = ["data.csv"]
        result = self.analyzer.analyze(question, attachments)
        
        assert 'file_handler' in result['required_tools']
        assert result['complexity'] in ['low', 'medium', 'high']
    
    # Add more test cases as implementation progresses