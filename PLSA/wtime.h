/* C/Fortran interface is different on different machines. 
 * You may need to tweak this.
 */

void timer_clear(int n);
void timer_start(int n);
void timer_stop(int n);
double timer_read(int n);
