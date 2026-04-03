/* ============================================================================
 * Eating the CPU
 * ============================================================================
 * A tight infinite loop does not idle; it burns processing cycles as fast as the CPU clock allows. Early UNIX time-sharing systems heavily punished processes like this by ruthlessly downgrading their scheduling priority so interactive users wouldn't suffer lag.
 * ============================================================================
 */
int main() {
	int n;
	while(1) {
		n++;
	}
	return n;
}
