/* ============================================================================
 * Measuring the Weight of the OS
 * ============================================================================
 * The 'load average' (e.g. 1.05, 1.10, 0.95) was invented to give sysadmins a quick glance at system health. It measures the exponential moving average of processes either currently running or stuck waiting for the disk. This file directly queries that historic kernel metric.
 * ============================================================================
 */
/* Similar to what uptime(1) or w(1) print. */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#define INTERVALS 3

int
main() {
	double loadavg[INTERVALS];

	if (getloadavg(loadavg, INTERVALS) == -1) {
		err(EXIT_FAILURE, "getloadavg");
		/* NOTREACHED */
	}

	(void)printf("%lf %lf %lf\n", loadavg[0], loadavg[1], loadavg[2]);
	return EXIT_SUCCESS;
}
