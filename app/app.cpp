extern "C" {
    void web_test_function ();
}

void web_test_function () {

}

void initialize ();

bool recive_chunk (void *chunk, unsigned int chunk_len);

void set_clbck_on_image (void (*)(char *image_buffer, unsigned int image_size));
