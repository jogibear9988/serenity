/*
 * Copyright (c) 2023, Matthew Olsson <mattco@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Optional.h>
#include <AK/SinglyLinkedList.h>
#include <LibWeb/Bindings/PlatformObject.h>
#include <LibWeb/Streams/AbstractOperations.h>

namespace Web::Streams {

enum class ReaderType {
    Default,
    Byob,
    None,
};

// https://streams.spec.whatwg.org/#pull-into-descriptor
struct PullIntoDescriptor {
    // https://streams.spec.whatwg.org/#pull-into-descriptor-buffer
    // An ArrayBuffer
    JS::NonnullGCPtr<JS::ArrayBuffer> buffer;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-buffer-byte-length
    // A positive integer representing the initial byte length of buffer
    u32 buffer_byte_length;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-byte-offset
    // A nonnegative integer byte offset into the buffer where the underlying byte source will start writing
    u32 byte_offset;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-byte-length
    // A positive integer number of bytes which can be written into the buffer
    u32 byte_length;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-bytes-filled
    // A nonnegative integer number of bytes that have been written into the buffer so far
    u32 bytes_filled;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-element-size
    // A positive integer representing the number of bytes that can be written into the buffer at a time, using views of the type described by the view constructor
    u32 element_size;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-view-constructor
    // A typed array constructor or %DataView%, which will be used for constructing a view with which to write into the buffer
    JS::NonnullGCPtr<JS::NativeFunction> view_constructor;

    // https://streams.spec.whatwg.org/#pull-into-descriptor-reader-type
    // Either "default" or "byob", indicating what type of readable stream reader initiated this request, or "none" if the initiating reader was released
    ReaderType reader_type;
};

// https://streams.spec.whatwg.org/#readable-byte-stream-queue-entry
struct ReadableByteStreamQueueEntry {
    // https://streams.spec.whatwg.org/#readable-byte-stream-queue-entry-buffer
    // An ArrayBuffer, which will be a transferred version of the one originally supplied by the underlying byte source
    JS::NonnullGCPtr<JS::ArrayBuffer> buffer;

    // https://streams.spec.whatwg.org/#readable-byte-stream-queue-entry-byte-offset
    // A nonnegative integer number giving the byte offset derived from the view originally supplied by the underlying byte source
    u32 byte_offset;

    // https://streams.spec.whatwg.org/#readable-byte-stream-queue-entry-byte-length
    // A nonnegative integer number giving the byte length derived from the view originally supplied by the underlying byte source
    u32 byte_length;
};

// https://streams.spec.whatwg.org/#readablebytestreamcontroller
class ReadableByteStreamController : public Bindings::PlatformObject {
    WEB_PLATFORM_OBJECT(ReadableByteStreamController, Bindings::PlatformObject);

public:
    virtual ~ReadableByteStreamController() override = default;

    JS::GCPtr<ReadableStreamBYOBRequest> byob_request() { return m_byob_request; }
    void set_byob_request(JS::GCPtr<ReadableStreamBYOBRequest> request) { m_byob_request = request; }

    Optional<double> desired_size() const;

    Optional<u32> const& auto_allocate_chunk_size() { return m_auto_allocate_chunk_size; }
    void set_auto_allocate_chunk_size(Optional<u32> value) { m_auto_allocate_chunk_size = value; }

    auto& cancel_algorithm() { return m_cancel_algorithm; }
    void set_cancel_algorithm(Optional<CancelAlgorithm> value) { m_cancel_algorithm = move(value); }

    bool close_requested() const { return m_close_requested; }
    void set_close_requested(bool value) { m_close_requested = value; }

    bool pull_again() const { return m_pull_again; }
    void set_pull_again(bool value) { m_pull_again = value; }

    auto& pull_algorithm() { return m_pull_algorithm; }
    void set_pull_algorithm(Optional<PullAlgorithm> value) { m_pull_algorithm = move(value); }

    bool pulling() const { return m_pulling; }
    void set_pulling(bool value) { m_pulling = value; }

    SinglyLinkedList<PullIntoDescriptor>& pending_pull_intos() { return m_pending_pull_intos; }

    SinglyLinkedList<ReadableByteStreamQueueEntry>& queue() { return m_queue; }

    u32 queue_total_size() const { return m_queue_total_size; }
    void set_queue_total_size(u32 size) { m_queue_total_size = size; }

    bool started() const { return m_started; }
    void set_started(bool value) { m_started = value; }

    u32 strategy_hwm() const { return m_strategy_hwm; }
    void set_strategy_hwm(u32 value) { m_strategy_hwm = value; }

    JS::GCPtr<ReadableStream const> stream() const { return m_stream; }
    JS::GCPtr<ReadableStream> stream() { return m_stream; }
    void set_stream(JS::GCPtr<ReadableStream> stream) { m_stream = stream; }

    WebIDL::ExceptionOr<JS::NonnullGCPtr<WebIDL::Promise>> cancel_steps(JS::Value reason);
    WebIDL::ExceptionOr<void> pull_steps(NonnullRefPtr<ReadRequest>);
    WebIDL::ExceptionOr<void> release_steps();

private:
    explicit ReadableByteStreamController(JS::Realm&);

    virtual void visit_edges(Cell::Visitor&) override;

    // https://streams.spec.whatwg.org/#readablebytestreamcontroller-autoallocatechunksize
    // A positive integer, when the automatic buffer allocation feature is enabled. In that case, this value specifies the size of buffer to allocate. It is undefined otherwise.
    Optional<u32> m_auto_allocate_chunk_size;

    // https://streams.spec.whatwg.org/#readablebytestreamcontroller-byobrequest
    // A ReadableStreamBYOBRequest instance representing the current BYOB pull request, or null if there are no pending requests
    JS::GCPtr<ReadableStreamBYOBRequest> m_byob_request;

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-cancelalgorithm
    // A promise-returning algorithm, taking one argument (the cancel reason), which communicates a requested cancelation to the underlying source
    Optional<CancelAlgorithm> m_cancel_algorithm;

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-closerequested
    // A boolean flag indicating whether the stream has been closed by its underlying source, but still has chunks in its internal queue that have not yet been read
    bool m_close_requested { false };

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-pullagain
    // A boolean flag set to true if the stream’s mechanisms requested a call to the underlying source's pull algorithm to pull more data, but the pull could not yet be done since a previous call is still executing
    bool m_pull_again { false };

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-pullalgorithm
    // A promise-returning algorithm that pulls data from the underlying source
    Optional<PullAlgorithm> m_pull_algorithm;

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-pulling
    // A boolean flag set to true while the underlying source's pull algorithm is executing and the returned promise has not yet fulfilled, used to prevent reentrant calls
    bool m_pulling { false };

    // https://streams.spec.whatwg.org/#readablebytestreamcontroller-pendingpullintos
    // A list of pull-into descriptors
    SinglyLinkedList<PullIntoDescriptor> m_pending_pull_intos;

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-queue
    // A list representing the stream’s internal queue of chunks
    SinglyLinkedList<ReadableByteStreamQueueEntry> m_queue;

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-queuetotalsize
    // The total size of all the chunks stored in [[queue]]
    u32 m_queue_total_size { 0 };

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-started
    // A boolean flag indicating whether the underlying source has finished starting
    bool m_started { false };

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-strategyhwm
    // A number supplied to the constructor as part of the stream’s queuing strategy, indicating the point at which the stream will apply backpressure to its underlying source
    u32 m_strategy_hwm { 0 };

    // https://streams.spec.whatwg.org/#readablestreamdefaultcontroller-stream
    // The ReadableStream instance controlled
    JS::GCPtr<ReadableStream> m_stream;
};

}
