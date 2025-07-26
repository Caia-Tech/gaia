import { GaiaCircuit, CircuitConfig } from './base.js';
export declare class AdaptiveTopology extends GaiaCircuit {
    private adaptiveGates;
    private generation;
    private bestFitness;
    private mutationLog;
    constructor(name: string, config?: CircuitConfig & {
        mutationRate?: number;
    });
    private initializeAdaptive;
    execute(inputs: number[]): number[];
    evolve(fitnessFunction: string, iterations: number): string;
    private generateTestInputs;
    private generateMutations;
    private applyMutations;
    private addRandomGate;
    private removeGate;
    private mutateGateType;
    private rewireRandomConnection;
    getEvolutionStats(): any;
    getType(): string;
    visualize(): string;
}
//# sourceMappingURL=adaptive.d.ts.map