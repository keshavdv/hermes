/*
 * Copyright 2015, Keshav Varma
 * All Rights Reserved.
 */

#ifndef ENCODE_H
#define ENCODE_H

bool encode_state(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool encode_payload(pb_ostream_t *stream, const pb_field_t messagetype[], const void *message);

#endif
