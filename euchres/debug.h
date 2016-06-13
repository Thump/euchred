

/* Debug levels: by setting vl (verbosity level) to the logical or
 * of a set of these values, and setting debug true, the operation
 * 'debug(VALUE) cmd' will execute command if VALUE was among the
 * values ored together to produce vl.  Clear?  Bon.
 *
 * If debug is turned on but verbosity is set to 0, it will not turn
 * on extra messages, but all log messages will be redirected to the
 * the screen.  Woo hoo, a beauty mark!
 *
 * To add a debug level, put in the #define, and set it equal to twice
 * the largest value already present.  
 */

/* turns function entry logging on */
#define GENERAL      1

/* turns on data send logging on */
#define SEND      2
