
#include <libavformat/avformat.h>
#include <libavcodec/h264dec.h>

#include <memory.h>
#include <stdio.h>

/*
function jsCallback(ptr, size)   int ptr;
{
    // Access the WASM memory buffer
    const memory = new Uint8Array(wasmInstance.exports.memory.buffer, ptr, size);

    // Create an ArrayBuffer from the memory
    const arrayBuffer = memory.slice().buffer;

    console.log("Received ArrayBuffer:", arrayBuffer);

    // Example: Convert ArrayBuffer to string for demonstration
    const textDecoder = new TextDecoder();
    const message = textDecoder.decode(arrayBuffer);
    console.log("Message:", message);
}

let wasmInstance;

const importObject = {
    env: {
        jsCallback: (ptr, size) => jsCallback(ptr, size),
        memory: new WebAssembly.Memory({ initial: 1 }), // Ensure memory is available
    },
};
*/

extern void js_clbck (uint8_t* data, int size);

static AVFormatContext* input_format_context = NULL;

void process_chunk (uint8_t* buffer, int size) { // call it from JS on every mpegts chunk

    AVIOContext* avio_context = avio_alloc_context(buffer, size, 0, NULL, NULL, NULL, NULL);
    if (!avio_context) {
        printf("Failed to allocate AVIOContext\n");
        return;
    }

    // Read packets from the buffer
    AVPacket* packet = av_packet_alloc();

    int ret = av_read_frame(input_format_context, packet);
    if (ret < 0) {
        printf("Failed to read frame\n");
        av_packet_unref(packet);
        avio_context_free(&avio_context);
        return;
    }

    /* GOT VIDEO FREAME*/
    js_clbck(packet->data, packet->size);

    // Free the packet
    av_packet_unref(packet);

    // Free the AVIOContext
    avio_context_free(&avio_context);
}

static int e_code = 0;
#define E_CODE (--e_code)

int web_test_function () {
    
    // SHOULD BE QUEUE HERE
    uint8_t* buffer = calloc(sizeof(uint8_t), 5 * 1024 * 1024); // 1MB buffer
    if (!buffer) {
        return E_CODE;
    }

    int ret = avformat_open_input(&input_format_context, NULL, NULL, NULL);
    if (ret < 0) {
        printf("Failed to open input format context\n");
        return E_CODE;
    }

    // TODO: recieve chunk in buffer here and pass it 

    process_chunk(buffer, 0);

    avformat_close_input(&input_format_context);
    free(buffer);
    return E_CODE;
}
