#!/usr/bin/python

def distanceCm():
	f = open("/sys/class/hc_sr04/value",'r')
	t = f.read()	#microseconds
	f.close()
	if (long(t) == -1):
		return -1   #N/A
	else:
		return (float(t)/58)

def main():
	d = distanceCm()	
	print "%.1f cm" % d

if __name__ == "__main__":
    main() 
