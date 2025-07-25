#!/bin/bash

# Dataset download script for gaia text training
# Focus on high-quality, small datasets that fit in RAM

echo "Downloading high-quality datasets for gaia..."

# Create dataset directory
cd "$(dirname "$0")"

# 1. Open-Platypus (24.9k samples - excellent quality)
echo "Downloading Open-Platypus (24.9k high-quality samples)..."
curl -L "https://huggingface.co/datasets/garage-bAInd/Open-Platypus/resolve/main/data/train-00000-of-00001.parquet" -o platypus.parquet

# 2. SmolTalk subset (high quality conversational data)
echo "Downloading SmolTalk subset..."
curl -L "https://huggingface.co/datasets/HuggingFaceTB/smoltalk/resolve/main/data/train-00000-of-00004.parquet" -o smoltalk_0.parquet

# 3. Alpaca GPT-4 (52k samples - very high quality)
echo "Downloading Alpaca GPT-4 data..."
curl -L "https://raw.githubusercontent.com/Instruction-Tuning-with-GPT-4/GPT-4-LLM/main/data/alpaca_gpt4_data.json" -o alpaca_gpt4.json

# 4. High-quality Wikipedia subset (clean text)
echo "Downloading Wikipedia clean subset..."
curl -L "https://huggingface.co/datasets/wikipedia/resolve/main/data/20220301.simple/train-00000-of-00001.parquet" -o wikipedia_simple.parquet

# 5. TinyStories (high quality simple narratives)
echo "Downloading TinyStories subset..."
curl -L "https://huggingface.co/datasets/roneneldan/TinyStories/resolve/main/data/train-00000-of-00050.parquet" -o tinystories_0.parquet

echo "Download complete! Converting to text format..."

# Convert parquet files to text (requires pandas)
python3 << 'EOF'
import json
import os

def extract_text_samples(max_samples=10000):
    """Extract high-quality text samples for gaia training"""
    
    output_texts = []
    
    # Process Alpaca GPT-4 JSON
    if os.path.exists('alpaca_gpt4.json'):
        print("Processing Alpaca GPT-4...")
        with open('alpaca_gpt4.json', 'r') as f:
            data = json.load(f)
            for item in data[:5000]:  # First 5k samples
                text = f"{item.get('instruction', '')} {item.get('input', '')} {item.get('output', '')}"
                output_texts.append(text.strip())
    
    # Try to process parquet files if pandas available
    try:
        import pandas as pd
        
        # Process Open-Platypus
        if os.path.exists('platypus.parquet'):
            print("Processing Open-Platypus...")
            df = pd.read_parquet('platypus.parquet')
            for _, row in df.head(5000).iterrows():
                if 'input' in row and 'output' in row:
                    text = f"{row['input']} {row['output']}"
                    output_texts.append(text.strip())
        
        # Process SmolTalk
        if os.path.exists('smoltalk_0.parquet'):
            print("Processing SmolTalk...")
            df = pd.read_parquet('smoltalk_0.parquet')
            for _, row in df.head(2000).iterrows():
                if 'text' in row:
                    output_texts.append(row['text'].strip())
                    
    except ImportError:
        print("pandas not available - processing JSON only")
    
    # Write high-quality samples
    with open('high_quality_text.txt', 'w') as f:
        for text in output_texts[:max_samples]:
            if len(text) > 10:  # Filter short texts
                f.write(text + '\n\n')
    
    print(f"Extracted {len(output_texts)} text samples")
    
extract_text_samples()
EOF

echo "Dataset preparation complete!"
echo "High-quality text saved to: datasets/high_quality_text.txt"