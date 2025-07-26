// Base GAIA Circuit class
export interface Gate {
  id: string;
  type: 'AND' | 'OR' | 'NOT' | 'XOR' | 'NAND' | 'NOR' | 'THRESHOLD';
  inputs: string[];
  output: number;
  threshold?: number;
}

export interface CircuitConfig {
  gates?: Gate[];
  connections?: Map<string, string[]>;
  inputCount?: number;
  outputCount?: number;
}

export class GaiaCircuit {
  protected name: string;
  protected gates: Map<string, Gate>;
  protected connections: Map<string, string[]>;
  protected executionTrace: string[];
  protected inputCount: number;
  protected outputCount: number;

  constructor(name: string, config: CircuitConfig = {}) {
    this.name = name;
    this.gates = new Map();
    this.connections = config.connections || new Map();
    this.executionTrace = [];
    this.inputCount = config.inputCount || 2;
    this.outputCount = config.outputCount || 1;

    // Initialize with provided gates or create default
    if (config.gates) {
      config.gates.forEach(gate => this.gates.set(gate.id, gate));
    } else {
      this.createDefaultCircuit();
    }
  }

  protected createDefaultCircuit(): void {
    // Simple AND gate by default
    this.gates.set('G1', {
      id: 'G1',
      type: 'AND',
      inputs: ['IN0', 'IN1'],
      output: 0,
    });
    this.connections.set('G1', ['OUT0']);
  }

  execute(inputs: number[]): number[] {
    this.executionTrace = [`Executing ${this.name} with inputs: [${inputs}]`];
    
    // Create input values map
    const values = new Map<string, number>();
    inputs.forEach((val, idx) => {
      values.set(`IN${idx}`, val);
      this.executionTrace.push(`  IN${idx} = ${val}`);
    });

    // Topological sort for execution order
    const sorted = this.topologicalSort();
    
    // Execute gates in order
    for (const gateId of sorted) {
      const gate = this.gates.get(gateId);
      if (!gate) continue;

      const inputValues = gate.inputs.map(input => values.get(input) || 0);
      gate.output = this.evaluateGate(gate, inputValues);
      values.set(gateId, gate.output);
      
      this.executionTrace.push(`  ${gateId} (${gate.type}) [${inputValues}] → ${gate.output}`);
    }

    // Collect outputs
    const outputs: number[] = [];
    for (let i = 0; i < this.outputCount; i++) {
      const outputGate = Array.from(this.connections.entries())
        .find(([_, conns]) => conns.includes(`OUT${i}`));
      
      if (outputGate) {
        outputs.push(values.get(outputGate[0]) || 0);
      } else {
        outputs.push(0);
      }
    }

    this.executionTrace.push(`  Outputs: [${outputs}]`);
    return outputs;
  }

  protected evaluateGate(gate: Gate, inputs: number[]): number {
    switch (gate.type) {
      case 'AND':
        return inputs.every(v => v === 1) ? 1 : 0;
      case 'OR':
        return inputs.some(v => v === 1) ? 1 : 0;
      case 'NOT':
        return inputs[0] === 1 ? 0 : 1;
      case 'XOR':
        return inputs.filter(v => v === 1).length % 2;
      case 'NAND':
        return inputs.every(v => v === 1) ? 0 : 1;
      case 'NOR':
        return inputs.some(v => v === 1) ? 0 : 1;
      case 'THRESHOLD':
        const sum = inputs.reduce((a, b) => a + b, 0);
        return sum >= (gate.threshold || 1) ? 1 : 0;
      default:
        return 0;
    }
  }

  protected topologicalSort(): string[] {
    const visited = new Set<string>();
    const result: string[] = [];

    const visit = (nodeId: string) => {
      if (visited.has(nodeId)) return;
      visited.add(nodeId);

      const gate = this.gates.get(nodeId);
      if (gate) {
        // Visit dependencies first
        gate.inputs.forEach(input => {
          if (this.gates.has(input)) {
            visit(input);
          }
        });
      }

      result.push(nodeId);
    };

    // Visit all gates
    this.gates.forEach((_, gateId) => visit(gateId));

    return result;
  }

  visualize(): string {
    let viz = `Circuit: ${this.name}\n`;
    viz += `Type: ${this.getType()}\n`;
    viz += `Gates: ${this.gates.size}\n\n`;

    viz += 'Structure:\n';
    this.gates.forEach((gate, id) => {
      viz += `  ${id} (${gate.type})\n`;
      viz += `    Inputs: ${gate.inputs.join(', ')}\n`;
      const connections = this.connections.get(id) || [];
      viz += `    Outputs to: ${connections.join(', ')}\n`;
    });

    return viz;
  }

  getType(): string {
    return 'basic';
  }

  getGateCount(): number {
    return this.gates.size;
  }

  getExecutionTrace(): string {
    return this.executionTrace.join('\n');
  }

  serialize(): any {
    return {
      name: this.name,
      type: this.getType(),
      gates: Array.from(this.gates.values()),
      connections: Array.from(this.connections.entries()),
      inputCount: this.inputCount,
      outputCount: this.outputCount,
    };
  }

  addGate(gate: Gate): void {
    this.gates.set(gate.id, gate);
  }

  connectGates(from: string, to: string): void {
    const connections = this.connections.get(from) || [];
    connections.push(to);
    this.connections.set(from, connections);
  }
}