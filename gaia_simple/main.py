"""
Main Entry Point for GAIA Simple

Provides the main interface for solving GAIA benchmark questions.
"""

import asyncio
from typing import Dict, Any, List, Optional
from pathlib import Path

from core.question_analyzer import QuestionAnalyzer
from core.task_planner import TaskPlanner
from core.executor import TaskExecutor
from core.answer_generator import AnswerGenerator
from tools.tool_manager import ToolManager
from tools.web_search import WebSearchTool
from tools.code_executor import CodeExecutorTool
from tools.file_handler import FileHandlerTool
from tools.calculator import CalculatorTool
from utils.config import Config
from utils.logger import logger


class GAIASolver:
    """Main solver for GAIA benchmark questions."""
    
    def __init__(self, config_path: Optional[str] = None):
        self.config = Config(config_path)
        self.logger = logger
        
        # Initialize components
        self.question_analyzer = QuestionAnalyzer()
        self.task_planner = TaskPlanner()
        self.tool_manager = self._initialize_tools()
        self.executor = TaskExecutor(self.tool_manager)
        self.answer_generator = AnswerGenerator()
    
    def _initialize_tools(self) -> ToolManager:
        """Initialize and register all tools."""
        manager = ToolManager()
        
        # Register tools based on configuration
        if self.config.get("tools.web_search.enabled"):
            manager.register_tool(WebSearchTool(
                api_key=self.config.get("tools.web_search.api_key")
            ))
        
        if self.config.get("tools.code_executor.enabled"):
            manager.register_tool(CodeExecutorTool(
                timeout=self.config.get("tools.code_executor.timeout", 30)
            ))
        
        if self.config.get("tools.file_handler.enabled"):
            manager.register_tool(FileHandlerTool())
        
        # Calculator is always enabled
        manager.register_tool(CalculatorTool())
        
        return manager
    
    async def solve(self, question: str, attachments: List[str] = None) -> Dict[str, Any]:
        """
        Solve a GAIA benchmark question.
        
        Args:
            question: The question text
            attachments: List of attachment file paths
            
        Returns:
            Dictionary containing:
            - answer: The final answer
            - explanation: Step-by-step explanation
            - confidence: Confidence score
            - execution_log: Detailed execution log
        """
        self.logger.info(f"Starting to solve question: {question[:100]}...")
        
        try:
            # Step 1: Analyze the question
            self.logger.log_step("Question Analysis", "started")
            analysis = self.question_analyzer.analyze(question, attachments)
            self.logger.log_step("Question Analysis", "completed", 
                               f"Type: {analysis.get('question_type', 'unknown')}")
            
            # Step 2: Create execution plan
            self.logger.log_step("Task Planning", "started")
            plan = self.task_planner.create_plan(analysis)
            self.logger.log_step("Task Planning", "completed", 
                               f"Steps: {len(plan)}")
            
            # Step 3: Execute the plan
            self.logger.log_step("Plan Execution", "started")
            execution_results = await self.executor.execute_plan(plan)
            self.logger.log_step("Plan Execution", "completed",
                               f"Successful steps: {sum(1 for r in execution_results if r.success)}/{len(execution_results)}")
            
            # Step 4: Generate answer
            self.logger.log_step("Answer Generation", "started")
            result = self.answer_generator.generate_answer(
                question, 
                execution_results,
                analysis.get('answer_type', 'text')
            )
            self.logger.log_step("Answer Generation", "completed")
            
            # Add execution details
            result['execution_log'] = self._format_execution_log(execution_results)
            
            self.logger.info(f"Successfully solved question. Answer: {result['answer']}")
            return result
            
        except Exception as e:
            self.logger.error(f"Error solving question: {str(e)}")
            return {
                "answer": None,
                "explanation": f"Error occurred: {str(e)}",
                "confidence": 0.0,
                "execution_log": []
            }
    
    def _format_execution_log(self, execution_results: List) -> List[Dict[str, Any]]:
        """Format execution results for output."""
        log = []
        for result in execution_results:
            log.append({
                "step_id": result.step_id,
                "success": result.success,
                "output": str(result.output)[:500],  # Truncate long outputs
                "error": result.error
            })
        return log
    
    def solve_sync(self, question: str, attachments: List[str] = None) -> Dict[str, Any]:
        """Synchronous wrapper for solve method."""
        return asyncio.run(self.solve(question, attachments))


def main():
    """Command-line interface for GAIA Simple."""
    import argparse
    
    parser = argparse.ArgumentParser(description="GAIA Simple - Solve GAIA benchmark questions")
    parser.add_argument("question", nargs="?", help="Question to solve")
    parser.add_argument("--file", "-f", help="Path to question file")
    parser.add_argument("--attachments", "-a", nargs="*", help="Attachment files")
    parser.add_argument("--config", "-c", help="Configuration file path")
    
    args = parser.parse_args()
    
    # Get question text
    if args.question:
        question = args.question
    elif args.file:
        with open(args.file, 'r') as f:
            question = f.read().strip()
    else:
        print("Please provide a question either directly or via --file")
        return
    
    # Initialize solver
    solver = GAIASolver(config_path=args.config)
    
    # Solve question
    result = solver.solve_sync(question, attachments=args.attachments)
    
    # Print results
    print("\n" + "="*50)
    print("ANSWER:", result['answer'])
    print("\nEXPLANATION:")
    print(result['explanation'])
    print(f"\nCONFIDENCE: {result['confidence']:.2f}")
    print("="*50)


if __name__ == "__main__":
    main()