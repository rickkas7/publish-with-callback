# Publish with Callback

One annoying thing about the [Particle.publish](https://docs.particle.io/reference/device-os/firmware/electron/#particle-publish-) API is that it is blocking, if you want to know if the publish succeeded or not. 

Sometimes you want to fire off the publish, and do some other stuff while it's processing, but still know if it succeeded or not. This code allow you to do it easily. 

This is just a single .h file you can include in your project; it's not a full library.

Here's the sample code:

```
#include "PublishCallback.h"


SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

const unsigned long PUBLISH_INTERVAL_MS = 60000;
unsigned long lastPublish = 0;

PublishCallback pubCallback;
int counter = 0;

void setup() {
	pubCallback.withCallback([](int err, const void *) {
		Log.info("callback called! err=%d", err);
	});
}

void loop() {
	if (millis() - lastPublish >= PUBLISH_INTERVAL_MS) {
		lastPublish = millis();

		if (Particle.connected()) {
			char buf[64];
			snprintf(buf, sizeof(buf), "counter=%d", ++counter);

			Log.info("about to publish %s", buf);
			pubCallback.publish("testEvent", buf, PRIVATE, WITH_ACK);
		}
	}
}

```

Add the include. All of the code is inside that .h file, there's no separate .cpp file or library.

```
#include "PublishCallback.h"
```

You need to allocate a `PublishCallback` object. This can't be on the stack in loop or any function you use to publish because the object must live until the publish is complete. A global variable is a good place to put it.

```
PublishCallback pubCallback;
```

You need to set up the callback. It's done here using a C++11 lambda, but you can just pass a function pointer if you prefer. It's done here from setup, but you could do it elsewhere if you preferred. Since it uses std::function internally, your callback can also be a class member function.

```
pubCallback.withCallback([](int err, const void *) {
	Log.info("callback called! err=%d", err);
});
```

In this case, Log.info is called on every completion. The code inside the lambda is executed later, not during setup.

When you publish, you simply use pubCallback.publish instead of the normal Particle.publish. You probably want to WITH_ACK, because the whole point is to know if the publish succeeded.

```
pubCallback.publish("testEvent", buf, PRIVATE, WITH_ACK);
```

## Logs

### Success

If you're monitoring the serial debug log, a successful publish looks like this:

```
0000090000 [app] INFO: about to publish counter=9
0000090000 [comm.coap] TRACE: sending message id=2f
0000090000 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
    89.999 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
    92.310 AT read  >    3 "\r\n@"
    92.310 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00R\x00*\x00\x01\x00\x00\x00\x00\x00RG\xca\x14\x06Hg\xc4\xe6\x7f4\xe0\xff[l\x8f&\x97x\x95h\x80\x92w\xc66\xbd\xe6\x98\xb5\xb3N\xbd\t\x93"
    92.448 AT read  +   16 "\r\n+USOST: 0,55\r\n"
    92.449 AT read OK    6 "\r\nOK\r\n"
    92.449 AT send      14 "AT+USORF=0,0\r\n"
    92.456 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
    92.457 AT read OK    6 "\r\nOK\r\n"
    94.788 AT read  +   17 "\r\n+UUSORD: 0,33\r\n"
Socket 0: handle 0 has 33 bytes pending
    94.799 AT send      17 "AT+USORF=0,1024\r\n"
    94.810 AT read  +   70 "\r\n+USORF: 0,\"65.19.178.42\",5684,33,\"\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00t\x00\x14\x00\x01\x00\x00\x00\x00\x00t\x01~\xccp\x83Sn;\xfe\xeb\x8f\xc0\""
    94.815 AT read OK    6 "\r\nOK\r\n"
    94.815 AT send      14 "AT+USORF=0,0\r\n"
    94.820 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
    94.821 AT read OK    6 "\r\nOK\r\n"
0000094823 [system] TRACE: received 33
0000094823 [comm.coap] TRACE: recieved ACK for message id=2f
0000094825 [app] INFO: callback called! err=0
0000094825 [comm.protocol] INFO: rcv'd message type=13
```

The err passed to your callback will be 0 (Error::NONE) on a successful send and ACK.


### Timeout

In the case of the publish getting lost and timing out, you'll see something like:

```
0000060000 [app] INFO: about to publish counter=1
0000060000 [comm.coap] TRACE: sending message id=48
0000060000 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
    60.001 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
    61.703 AT read  >    3 "\r\n@"
    61.703 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00x\x00*\x00\x01\x00\x00\x00\x00\x00x\x12z\xdb\x80\x9ba\x14\xff\v\xa1\xee\xa48\xf9d0oY\x9cx!\x84Y\xb7\x81\xd5\x1e\x8d\xd2\xe4:\xa1q\x8c"
    61.841 AT read  +   16 "\r\n+USOST: 0,55\r\n"
    61.842 AT read OK    6 "\r\nOK\r\n"
    61.842 AT send      14 "AT+USORF=0,0\r\n"
    61.849 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
    61.850 AT read OK    6 "\r\nOK\r\n"
0000064414 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
    64.414 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
    64.422 AT read  >    3 "\r\n@"
    64.422 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00y\x00*\x00\x01\x00\x00\x00\x00\x00y\x16B\xa3nx\x00\xd9\xa8\x15\xb69\xc6\r\xfa\x0e65\xec\x1fuY&\x8b\xfb\x1aa\x9buf\x89\xb4\xfb\xf6p"
    64.560 AT read  +   16 "\r\n+USOST: 0,55\r\n"
    64.561 AT read OK    6 "\r\nOK\r\n"
    64.561 AT send      14 "AT+USORF=0,0\r\n"
    64.567 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
    64.568 AT read OK    6 "\r\nOK\r\n"
    69.996 AT read  +   14 "\r\n+CIEV: 2,2\r\n"
0000072560 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
    72.560 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
    72.570 AT read  >    3 "\r\n@"
    72.570 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00z\x00*\x00\x01\x00\x00\x00\x00\x00zX\x01\xe1\xe2\xd7;\xa7\x87+\xea\xb7\xd4G!t\x90j\xa0\x87|\r\x05\x9e\xc3aY|\x01[!\xa0\t\xcbf"
    72.709 AT read  +   16 "\r\n+USOST: 0,55\r\n"
    72.710 AT read OK    6 "\r\nOK\r\n"
    72.710 AT send      14 "AT+USORF=0,0\r\n"
    72.716 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
    72.717 AT read OK    6 "\r\nOK\r\n"
0000079921 [app] INFO: callback called! err=-160
``` 

Of note, the callback is called with error -160, which is Error::TIMEOUT.

The full list of system error codes can be found [here](https://github.com/particle-iot/device-os/blob/develop/services/inc/system_error.h).

### Packet loss

In the case where the first two attempts to send the publish fail, but then succeeds on the third, the log would look like this:

```
0000420000 [app] INFO: about to publish counter=7
0000420000 [comm.coap] TRACE: sending message id=51
0000420000 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
   420.001 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
   422.337 AT read  >    3 "\r\n@"
   422.337 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00\f\x00*\x00\x01\x00\x00\x00\x00\x00\f\xe0+\xb1K\x0f\xd8\x99\xf4F\xc6\r$:\xce\x1a(\x19\xd8\xc8\xb3\xcb:\x8bqS\xd6`\xe4\xb6\xa1\xde\xfd*\x9d"
   422.475 AT read  +   16 "\r\n+USOST: 0,55\r\n"
   422.476 AT read OK    6 "\r\nOK\r\n"
   422.476 AT send      14 "AT+USORF=0,0\r\n"
   422.483 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
   422.484 AT read OK    6 "\r\nOK\r\n"
0000425048 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
   425.048 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
   425.056 AT read  >    3 "\r\n@"
   425.056 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00\r\x00*\x00\x01\x00\x00\x00\x00\x00\rs\x96%\xca\x93cu\x1b\x8d\xcb=J+\xf6\x87|\xe6c\x93\xed%\xd1\x00\xb6qm\x93\xd6A\xfc\x9b\xe8\x0e\x81"
   425.194 AT read  +   16 "\r\n+USOST: 0,55\r\n"
   425.195 AT read OK    6 "\r\nOK\r\n"
   425.195 AT send      14 "AT+USORF=0,0\r\n"
   425.200 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
   425.201 AT read OK    6 "\r\nOK\r\n"
0000435413 [system] TRACE: send 55
socketSendTo(0,65.19.178.42,5684,,55)
   435.413 AT send      35 "AT+USOST=0,\"65.19.178.42\",5684,55\r\n"
   436.508 AT read  +   14 "\r\n+CIEV: 2,3\r\n"
   436.512 AT read  >    3 "\r\n@"
   436.512 AT send      55 "\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00\x0e\x00*\x00\x01\x00\x00\x00\x00\x00\x0e\xc1\x12m\xe4G\xa8\xe86\x180\n\xd1\x18\xf9\xbdx\xbd:\x9f\xa6\xeb?\x850\xe3Y\xae\xda\xa7\xd9#S\xc1\xfd"
   436.652 AT read  +   16 "\r\n+USOST: 0,55\r\n"
   436.653 AT read OK    6 "\r\nOK\r\n"
   436.653 AT send      14 "AT+USORF=0,0\r\n"
   436.662 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
   436.663 AT read OK    6 "\r\nOK\r\n"
   437.318 AT read  +   14 "\r\n+CIEV: 2,2\r\n"
   438.872 AT read  +   17 "\r\n+UUSORD: 0,33\r\n"
Socket 0: handle 0 has 33 bytes pending
   438.883 AT send      17 "AT+USORF=0,1024\r\n"
   438.894 AT read  +   70 "\r\n+USORF: 0,\"65.19.178.42\",5684,33,\"\x17\xfe\xfd\x00\x01\x00\x00\x00\x00\x00\f\x00\x14\x00\x01\x00\x00\x00\x00\x00\f\x1e6\x95\xed\xa4q~\x8e\xe1\xaf\x8f\x8d\""
   438.899 AT read OK    6 "\r\nOK\r\n"
   438.899 AT send      14 "AT+USORF=0,0\r\n"
   438.904 AT read  +   15 "\r\n+USORF: 0,0\r\n"
Socket 0: handle 0 has 0 bytes pending
   438.905 AT read OK    6 "\r\nOK\r\n"
0000438905 [system] TRACE: received 33
0000438905 [comm.coap] TRACE: recieved ACK for message id=51
0000438907 [app] INFO: callback called! err=0
0000438907 [comm.protocol] INFO: rcv'd message type=13
```

## More info

You can find more about the oddities of Particle.publish blocking [here](https://github.com/rickkas7/particle-publish-blocking).

In order to test packet loss I use [electron-cloud-manipulator](https://github.com/rickkas7/electron-cloud-manipulator).
