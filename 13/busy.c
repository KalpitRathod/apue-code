/* ============================================================================
 * The Lone Monopolist
 * ============================================================================
 * A single-threaded loop calculating nothing, specifically designed to test the kernel's CPU limitation boundaries. It shows the impact of a runaway program on standard system responsiveness before advanced preemption features existed.
 * ============================================================================
 */
int
main() {
	int i = 0;
	while (1) {
		i++;
	}
}
