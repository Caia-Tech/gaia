import { GaiaCircuit, Gate, CircuitConfig } from './base.js';

interface AdaptiveGate extends Gate {
  fitness: number;
  mutationRate: number;
  activationCount: number;
}

interface Mutation {
  type: 'ADD_GATE' | 'REMOVE_GATE' | 'CHANGE_TYPE' | 'REWIRE';
  target?: string;
  details?: any;
}

export class AdaptiveTopology extends GaiaCircuit {
  private adaptiveGates: Map<string, AdaptiveGate>;
  private generation: number;
  private bestFitness: number;
  private mutationLog: string[];

  constructor(name: string, config: CircuitConfig & { mutationRate?: number } = {}) {
    super(name, config);
    this.adaptiveGates = new Map();
    this.generation = 0;
    this.bestFitness = 0;
    this.mutationLog = [];
    this.initializeAdaptive(config.mutationRate || 0.1);
  }

  private initializeAdaptive(mutationRate: number): void {
    this.gates.forEach((gate, id) => {
      this.adaptiveGates.set(id, {
        ...gate,
        fitness: 0,
        mutationRate,
        activationCount: 0,
      });
    });
  }

  execute(inputs: number[]): number[] {
    const result = super.execute(inputs);
    
    // Track activation counts for fitness
    this.adaptiveGates.forEach(gate => {
      if (gate.output === 1) {
        gate.activationCount++;
      }
    });
    
    return result;
  }

  evolve(fitnessFunction: string, iterations: number): string {
    const evalFitness = new Function('circuit', 'inputs', 'outputs', fitnessFunction);
    this.mutationLog = [`Starting evolution for ${iterations} generations`];
    
    for (let gen = 0; gen < iterations; gen++) {
      this.generation = gen + 1;
      
      // Test current configuration
      const testInputs = this.generateTestInputs();
      let totalFitness = 0;
      
      for (const inputs of testInputs) {
        const outputs = this.execute(inputs);
        const fitness = evalFitness(this, inputs, outputs);
        totalFitness += fitness;
      }
      
      const avgFitness = totalFitness / testInputs.length;
      
      if (avgFitness > this.bestFitness) {
        this.bestFitness = avgFitness;
        this.mutationLog.push(`Generation ${gen}: New best fitness = ${avgFitness.toFixed(3)}`);
      }
      
      // Apply mutations based on fitness
      if (avgFitness < 0.9) {  // Only mutate if not optimal
        const mutations = this.generateMutations(avgFitness);
        this.applyMutations(mutations);
      }
    }
    
    this.mutationLog.push(`Evolution complete. Best fitness: ${this.bestFitness.toFixed(3)}`);
    return this.mutationLog.join('\n');
  }

  private generateTestInputs(): number[][] {
    const inputs: number[][] = [];
    const numTests = Math.pow(2, this.inputCount);
    
    for (let i = 0; i < numTests; i++) {
      const input: number[] = [];
      for (let j = 0; j < this.inputCount; j++) {
        input.push((i >> j) & 1);
      }
      inputs.push(input);
    }
    
    return inputs;
  }

  private generateMutations(fitness: number): Mutation[] {
    const mutations: Mutation[] = [];
    const mutationProbability = (1 - fitness) * 0.3;  // Higher mutation rate for lower fitness
    
    // Potentially add a gate
    if (Math.random() < mutationProbability && this.gates.size < 20) {
      mutations.push({ type: 'ADD_GATE' });
    }
    
    // Potentially remove underperforming gates
    this.adaptiveGates.forEach((gate, id) => {
      if (gate.activationCount < this.generation * 0.1 && Math.random() < mutationProbability) {
        mutations.push({ type: 'REMOVE_GATE', target: id });
      }
    });
    
    // Potentially change gate types
    this.adaptiveGates.forEach((gate, id) => {
      if (Math.random() < mutationProbability * 0.5) {
        mutations.push({ type: 'CHANGE_TYPE', target: id });
      }
    });
    
    // Potentially rewire connections
    if (Math.random() < mutationProbability) {
      mutations.push({ type: 'REWIRE' });
    }
    
    return mutations;
  }

  private applyMutations(mutations: Mutation[]): void {
    mutations.forEach(mutation => {
      switch (mutation.type) {
        case 'ADD_GATE':
          this.addRandomGate();
          break;
          
        case 'REMOVE_GATE':
          if (mutation.target && this.gates.size > 1) {
            this.removeGate(mutation.target);
          }
          break;
          
        case 'CHANGE_TYPE':
          if (mutation.target) {
            this.mutateGateType(mutation.target);
          }
          break;
          
        case 'REWIRE':
          this.rewireRandomConnection();
          break;
      }
    });
  }

  private addRandomGate(): void {
    const newId = `G${this.gates.size + 1}`;
    const types: Gate['type'][] = ['AND', 'OR', 'NOT', 'XOR', 'NAND', 'NOR'];
    const type = types[Math.floor(Math.random() * types.length)];
    
    // Random inputs from existing gates or inputs
    const availableInputs = ['IN0', 'IN1', ...Array.from(this.gates.keys())];
    const numInputs = type === 'NOT' ? 1 : 2;
    const inputs: string[] = [];
    
    for (let i = 0; i < numInputs; i++) {
      inputs.push(availableInputs[Math.floor(Math.random() * availableInputs.length)]);
    }
    
    const newGate: AdaptiveGate = {
      id: newId,
      type,
      inputs,
      output: 0,
      fitness: 0,
      mutationRate: 0.1,
      activationCount: 0,
    };
    
    this.gates.set(newId, newGate);
    this.adaptiveGates.set(newId, newGate);
    
    // Connect to a random output or gate
    const targets = ['OUT0', ...Array.from(this.gates.keys()).filter(id => id !== newId)];
    const target = targets[Math.floor(Math.random() * targets.length)];
    this.connectGates(newId, target);
    
    this.mutationLog.push(`Added gate ${newId} (${type})`);
  }

  private removeGate(gateId: string): void {
    this.gates.delete(gateId);
    this.adaptiveGates.delete(gateId);
    this.connections.delete(gateId);
    
    // Remove references to this gate
    this.connections.forEach((conns, id) => {
      const filtered = conns.filter(conn => conn !== gateId);
      if (filtered.length !== conns.length) {
        this.connections.set(id, filtered);
      }
    });
    
    this.mutationLog.push(`Removed gate ${gateId}`);
  }

  private mutateGateType(gateId: string): void {
    const gate = this.adaptiveGates.get(gateId);
    if (!gate) return;
    
    const types: Gate['type'][] = ['AND', 'OR', 'NOT', 'XOR', 'NAND', 'NOR'];
    const currentIndex = types.indexOf(gate.type);
    const newType = types[(currentIndex + 1) % types.length];
    
    gate.type = newType;
    
    // Adjust inputs for NOT gate
    if (newType === 'NOT' && gate.inputs.length > 1) {
      gate.inputs = [gate.inputs[0]];
    } else if (gate.type !== 'NOT' && gate.inputs.length === 1) {
      // Add another input
      const availableInputs = ['IN0', 'IN1', ...Array.from(this.gates.keys())];
      gate.inputs.push(availableInputs[Math.floor(Math.random() * availableInputs.length)]);
    }
    
    this.mutationLog.push(`Mutated ${gateId} to ${newType}`);
  }

  private rewireRandomConnection(): void {
    const gateIds = Array.from(this.gates.keys());
    if (gateIds.length === 0) return;
    
    const sourceId = gateIds[Math.floor(Math.random() * gateIds.length)];
    const availableTargets = ['OUT0', ...gateIds.filter(id => id !== sourceId)];
    const newTarget = availableTargets[Math.floor(Math.random() * availableTargets.length)];
    
    this.connections.set(sourceId, [newTarget]);
    this.mutationLog.push(`Rewired ${sourceId} to ${newTarget}`);
  }

  getEvolutionStats(): any {
    return {
      generation: this.generation,
      bestFitness: this.bestFitness,
      gateCount: this.gates.size,
      activationStats: Array.from(this.adaptiveGates.entries()).map(([id, gate]) => ({
        id,
        type: gate.type,
        activationCount: gate.activationCount,
        activationRate: this.generation > 0 ? gate.activationCount / this.generation : 0,
      })),
    };
  }

  getType(): string {
    return 'adaptive';
  }

  visualize(): string {
    let viz = super.visualize();
    viz += `\nGeneration: ${this.generation}\n`;
    viz += `Best Fitness: ${this.bestFitness.toFixed(3)}\n`;
    viz += '\nGate Activity:\n';
    
    this.adaptiveGates.forEach((gate, id) => {
      const rate = this.generation > 0 ? gate.activationCount / this.generation : 0;
      viz += `  ${id}: ${(rate * 100).toFixed(1)}% activation rate\n`;
    });
    
    return viz;
  }
}