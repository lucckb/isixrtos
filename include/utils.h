/* ------------------------------------------------------------ */
/*
 * utils.h
 *
 *  Created on: 2010-03-26
 *      Author: lucck
 */
/* ------------------------------------------------------------ */
#ifndef UTILS_H_
#define UTILS_H_
/* ------------------------------------------------------------ */
#ifdef __cplusplus
 extern "C" {
#endif
/* ------------------------------------------------------------ */
//Convert string to unsigned
unsigned fnd_atoui(const char *str);
void fnd_uitoa(char *str, unsigned val , int fmt);
/* ------------------------------------------------------------ */
#ifdef __cplusplus
 }
#endif
/* ------------------------------------------------------------ */
#endif /* UTILS_H_ */
