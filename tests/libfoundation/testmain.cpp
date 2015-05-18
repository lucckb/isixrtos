/*
 * =====================================================================================
 *
 *       Filename:  testmain.cpp
 *
 *    Description: test main call 
 *
 *        Version:  1.0
 *        Created:  20.02.2015 23:23:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


int fstest_main( int argc, const char** /*  argv*/);
int libgsm_main( int argc, const char** /*  argv*/);

int main( int argc, const char* argv[] ) {
	return fstest_main( argc, argv );
}
