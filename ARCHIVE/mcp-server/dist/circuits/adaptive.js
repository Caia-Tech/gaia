import { GaiaCircuit } from './base.js';
export class AdaptiveTopology extends GaiaCircuit {
    adaptiveGates;
    generation;
    bestFitness;
    mutationLog;
    constructor(name, config = {}) {
        super(name, config);
        this.adaptiveGates = new Map();
        this.generation = 0;
        this.bestFitness = 0;
        this.mutationLog = [];
        this.initializeAdaptive(config.mutationRate || 0.1);
    }
    initializeAdaptive(mutationRate) {
        this.gates.forEach((gate, id) => {
            this.adaptiveGates.set(id, {
                ...gate,
                fitness: 0,
                mutationRate,
                activationCount: 0,
            });
        });
    }
    execute(inputs) {
        const result = super.execute(inputs);
        // Track activation counts for fitness
        this.adaptiveGates.forEach(gate => {
            if (gate.output === 1) {
                gate.activationCount++;
            }
        });
        return result;
    }
    evolve(fitnessFunction, iterations) {
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
            if (avgFitness < 0.9) { // Only mutate if not optimal
                const mutations = this.generateMutations(avgFitness);
                this.applyMutations(mutations);
            }
        }
        this.mutationLog.push(`Evolution complete. Best fitness: ${this.bestFitness.toFixed(3)}`);
        return this.mutationLog.join('\n');
    }
    generateTestInputs() {
        const inputs = [];
        const numTests = Math.pow(2, this.inputCount);
        for (let i = 0; i < numTests; i++) {
            const input = [];
            for (let j = 0; j < this.inputCount; j++) {
                input.push((i >> j) & 1);
            }
            inputs.push(input);
        }
        return inputs;
    }
    generateMutations(fitness) {
        const mutations = [];
        const mutationProbability = (1 - fitness) * 0.3; // Higher mutation rate for lower fitness
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
    applyMutations(mutations) {
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
    addRandomGate() {
        const newId = `G${this.gates.size + 1}`;
        const types = ['AND', 'OR', 'NOT', 'XOR', 'NAND', 'NOR'];
        const type = types[Math.floor(Math.random() * types.length)];
        // Random inputs from existing gates or inputs
        const availableInputs = ['IN0', 'IN1', ...Array.from(this.gates.keys())];
        const numInputs = type === 'NOT' ? 1 : 2;
        const inputs = [];
        for (let i = 0; i < numInputs; i++) {
            inputs.push(availableInputs[Math.floor(Math.random() * availableInputs.length)]);
        }
        const newGate = {
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
    removeGate(gateId) {
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
    mutateGateType(gateId) {
        const gate = this.adaptiveGates.get(gateId);
        if (!gate)
            return;
        const types = ['AND', 'OR', 'NOT', 'XOR', 'NAND', 'NOR'];
        const currentIndex = types.indexOf(gate.type);
        const newType = types[(currentIndex + 1) % types.length];
        gate.type = newType;
        // Adjust inputs for NOT gate
        if (newType === 'NOT' && gate.inputs.length > 1) {
            gate.inputs = [gate.inputs[0]];
        }
        else if (gate.type !== 'NOT' && gate.inputs.length === 1) {
            // Add another input
            const availableInputs = ['IN0', 'IN1', ...Array.from(this.gates.keys())];
            gate.inputs.push(availableInputs[Math.floor(Math.random() * availableInputs.length)]);
        }
        this.mutationLog.push(`Mutated ${gateId} to ${newType}`);
    }
    rewireRandomConnection() {
        const gateIds = Array.from(this.gates.keys());
        if (gateIds.length === 0)
            return;
        const sourceId = gateIds[Math.floor(Math.random() * gateIds.length)];
        const availableTargets = ['OUT0', ...gateIds.filter(id => id !== sourceId)];
        const newTarget = availableTargets[Math.floor(Math.random() * availableTargets.length)];
        this.connections.set(sourceId, [newTarget]);
        this.mutationLog.push(`Rewired ${sourceId} to ${newTarget}`);
    }
    getEvolutionStats() {
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
    getType() {
        return 'adaptive';
    }
    visualize() {
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
//# sourceMappingURL=adaptive.js.map