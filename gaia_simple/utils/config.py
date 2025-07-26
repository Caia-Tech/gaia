"""
Configuration Management

Handles configuration for the GAIA Simple system.
"""

import os
import json
from pathlib import Path
from typing import Dict, Any, Optional


class Config:
    """Configuration manager for GAIA Simple."""
    
    def __init__(self, config_path: Optional[str] = None):
        self.config_path = config_path or self._get_default_config_path()
        self.config = self._load_config()
    
    def _get_default_config_path(self) -> str:
        """Get default configuration file path."""
        return os.path.join(Path.home(), '.gaia_simple', 'config.json')
    
    def _load_config(self) -> Dict[str, Any]:
        """Load configuration from file."""
        if os.path.exists(self.config_path):
            with open(self.config_path, 'r') as f:
                return json.load(f)
        else:
            return self._get_default_config()
    
    def _get_default_config(self) -> Dict[str, Any]:
        """Get default configuration."""
        return {
            "model": {
                "provider": "openai",
                "name": "gpt-4",
                "temperature": 0.7,
                "max_tokens": 2000
            },
            "tools": {
                "web_search": {
                    "enabled": True,
                    "api_key": os.getenv("SEARCH_API_KEY", "")
                },
                "code_executor": {
                    "enabled": True,
                    "timeout": 30
                },
                "file_handler": {
                    "enabled": True,
                    "max_file_size": 10 * 1024 * 1024  # 10MB
                }
            },
            "execution": {
                "max_steps": 20,
                "timeout": 300,  # 5 minutes
                "retry_attempts": 3
            },
            "logging": {
                "level": "INFO",
                "file": "gaia_simple.log"
            }
        }
    
    def get(self, key: str, default: Any = None) -> Any:
        """Get configuration value by key."""
        keys = key.split('.')
        value = self.config
        for k in keys:
            if isinstance(value, dict) and k in value:
                value = value[k]
            else:
                return default
        return value
    
    def set(self, key: str, value: Any):
        """Set configuration value."""
        keys = key.split('.')
        config = self.config
        for k in keys[:-1]:
            if k not in config:
                config[k] = {}
            config = config[k]
        config[keys[-1]] = value
    
    def save(self):
        """Save configuration to file."""
        os.makedirs(os.path.dirname(self.config_path), exist_ok=True)
        with open(self.config_path, 'w') as f:
            json.dump(self.config, f, indent=2)