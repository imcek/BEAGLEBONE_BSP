/*
 * org_embedded_beaglebone_driver_kernel_chardev_BBNative.c
 *
 *  Created on: Jul 23, 2015
 *      Author: recep
 */
#include <stdio.h>
#include <jni.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include "org_embedded_beaglebone_driver_kernel_chardev_BBNative.h"
#include "BBNative.h"

int bbnative_fd=-1;

JNIEXPORT void JNICALL Java_org_embedded_beaglebone_driver_kernel_chardev_BBNative_gpioSetValue
  (JNIEnv * jniEnv, jobject jobj, jchar gpioNum, jchar gpioVal){
	char status[2];
	status[0]=gpioNum;
	status[1]=gpioVal;
	if(bbnative_fd!=-1) {
	   ioctl(bbnative_fd, GPIO_SET_VALUE, &status);
	}
	return;
}

JNIEXPORT void JNICALL Java_org_embedded_beaglebone_driver_kernel_chardev_BBNative_initialize
  (JNIEnv * jniEnv, jobject jobj){
	if(bbnative_fd==-1) {
		bbnative_fd = open(BBNATIVE_DEVICE_NAME, O_RDONLY);
	}
	return;
}

JNIEXPORT void JNICALL Java_org_embedded_beaglebone_driver_kernel_chardev_BBNative_shutDown
  (JNIEnv * jniEnv, jobject jobj){
	if(bbnative_fd!=-1)
	   close(bbnative_fd);
}

JNIEXPORT void JNICALL Java_org_embedded_beaglebone_driver_kernel_chardev_BBNative_sendLcdCommand
  (JNIEnv * jniEnv, jclass jcls, jchar cmd){
	if(bbnative_fd!=-1) {
	   ioctl(bbnative_fd, SEND_COMMAND, &cmd);
	}
	return;
}

/*
 * Class:     org_embedded_beaglebone_driver_kernel_chardev_BBNative
 * Method:    writeLcdLine
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_embedded_beaglebone_driver_kernel_chardev_BBNative_writeLcdLine
  (JNIEnv * jniEnv, jclass jcls, jstring str){

	const char *nativeString = (*jniEnv)->GetStringUTFChars(jniEnv, str, 0);
	if(bbnative_fd!=-1) {
	   ioctl(bbnative_fd, WRITE_LINE, nativeString);
	}
	(*jniEnv)->ReleaseStringUTFChars(jniEnv, str, nativeString);

	return;
}
