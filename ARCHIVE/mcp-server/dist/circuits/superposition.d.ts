import { GaiaCircuit, CircuitConfig } from './base.js';
export declare class SuperpositionGate extends GaiaCircuit {
    private superpositionGates;
    private collapseHistory;
    constructor(name: string, config?: CircuitConfig);
    private initializeSuperposition;
    execute(inputs: number[]): number[];
    private updateSuperposition;
    private calculateCertainty;
    private shouldCollapse;
    private collapseGate;
    private getMostProbableValue;
    private getStateDescription;
    observeStates(): any;
    getType(): string;
    visualize(): string;
}
//# sourceMappingURL=superposition.d.ts.map