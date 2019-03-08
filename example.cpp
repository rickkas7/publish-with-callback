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
			pubCallback.publish("testEvent", buf, PRIVATE | WITH_ACK);
		}
	}
}
