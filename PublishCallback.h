#ifndef __PUBLISHCALLBACK_H
#define __PUBLISHCALLBACK_H

#include "Particle.h"

/**
 * @brief Class to do a Particle.publish with an async callback, instead of blocking
 *
 * Requires Device OS 0.7.0 or later on the Electron, E Series, and Gen 3 (Argon, Boron, and Xenon).
 *
 * In general you should not allocate this object on the stack. The object must live until
 * the callback is called, so you don't want it to go out of scope and be deleted too soon.
 *
 * Official location: https://github.com/rickkas7/publish-with-callback
 * License: MIT
 */
class PublishCallback {
public:
	/**
	 * @brief Constructor with no parameters. Make sure you set the callback using withCallback.
	 */
	inline PublishCallback() : complete(false) {
		initEventData();
	}

	/**
	 * @brief Destructor
	 */
	inline ~PublishCallback() {
	}

	/**
	 * @brief Constructor that takes the callback function
	 */
	inline PublishCallback(std::function<void(int, const void *)> completion) : complete(false), completion(completion) {
		initEventData();
	}

	/**
	 * @brief Sets the callback for this object
	 *
	 * You can do this from setup(), like in the example, or you can pass the callback to the constructor.
	 *
	 * The callback function should have the prototype:
	 *
	 * ```
	 * void myCallback(int err, const void *data);
	 * ```
	 *
	 * The `err` is 0 on success (Error::NONE). The most common error code is -160 (Error::TIMEOUT).
	 *
	 * As the callback is specified using a std::function you can pass in a C++11 lambda, or a non-static class member function, as well.
	 */
	inline PublishCallback &withCallback(std::function<void(int, const void *)> completion) {
		this->completion = completion;
		return *this;
	}

    /**
     * @brief Overload of publish with no event data or TTL.
     *
     * @param eventName the event name to publish
     *
     * @param flags the publish flags like PUBLIC, PRIVATE, WITH_ACK, NO_ACK. You'll probably want to use PRIVATE | WITH_ACK.
     */
    inline void publish(const char *eventName, PublishFlags flags1, PublishFlags flags2 = PublishFlags())
    {
    	publish(eventName, NULL, flags1, flags2);
    }

    /**
     * @brief Overload of publish with no TTL, which isn't that useful anyway since it's ignored by the cloud
     *
     * @param eventName the event name to publish
     *
     * @param eventData the event data to publish
     *
     * @param flags the publish flags like PUBLIC, PRIVATE, WITH_ACK, NO_ACK. You'll probably want to use PRIVATE | WITH_ACK.
     */
    inline void publish(const char *eventName, const char *eventData, PublishFlags flags1, PublishFlags flags2 = PublishFlags())
    {
        publish(eventName, eventData, 60, flags1, flags2);
    }

    /**
     * @brief Overload of publish with all available parameters
     *
     * @param eventName the event name to publish
     *
     * @param eventData the event data to publish
     *
     * @param ttl the time to live for the event (currently ignored by the cloud)
     *
     * @param flags the publish flags like PUBLIC, PRIVATE, WITH_ACK, NO_ACK. You'll probably want to use PRIVATE | WITH_ACK.
     */
    inline void publish(const char *eventName, const char *eventData, int ttl, PublishFlags flags1, PublishFlags flags2 = PublishFlags())
    {
        publish_event(eventName, eventData, ttl, flags1 | flags2);
    }

    /**
     * @brief Internal version of publish, used by the other overloads
     *
     * @param eventName the event name to publish
     *
     * @param eventData the event data to publish
     *
     * @param ttl the time to live for the event (currently ignored by the cloud)
     *
     * @param flags the publish flags like PUBLIC, PRIVATE, WITH_ACK, NO_ACK. You'll probably want to use PRIVATE | WITH_ACK.
     */
    inline void publish_event(const char *eventName, const char *eventData, int ttl, PublishFlags flags) {

    	complete = false;

        if (!spark_send_event(eventName, eventData, ttl, flags.value(), this)) {
            // Set generic error code in case completion callback wasn't invoked for some reason
        	if (completion != NULL) {
        		completion(Error::UNKNOWN, NULL);
        	}
        }
    }

    /**
     * @brief Returns true if there is an outstanding publish that has not received an ack or timed out yet
     */
    inline bool isComplete() const {
    	return complete;
    }

private:
	inline void initEventData() {
		eventData.size = sizeof(spark_send_event_data);
		eventData.handler_callback = staticCallback;
		eventData.handler_data = this;
	}

    static inline void staticCallback(int error, const void* data, void* callbackData, void* reserved) {
    	PublishCallback *This = (PublishCallback *) callbackData;

    	if (This->completion != NULL) {
    		This->completion(error, data);
    	}
    	This->complete = true;
    }

    spark_send_event_data eventData;
    bool complete;
	std::function<void(int, const void *)> completion;
};

#endif /* __PUBLISHCALLBACK_H */
