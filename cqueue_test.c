#include <stdio.h>
#include "cqueue.h"

typedef struct {
	int other_val;
	int value;
} datum;

int main(int argc, char const *argv[])
{
	datum *d0, *d1;
	printf("Running test...\n");

	Queue *buffer = queue_initialize(sizeof(datum), 10);

	int count = 15;

	for(int j = 0; j < 2; j++) {
		for(int i = 0; i < count; i ++){
			d0 = (datum*) queue_enqueue(buffer);
			if(d0 == NULL) {
				printf("error enqueue at %d\n", i);
				continue;
			}
			d0->value = i*j;
			d0->other_val = -1*i;
		}

		for(int i = 0; i < count; i ++){
			int ret = queue_dequeue(buffer, (void**)&d1);
			if(ret == -1) {
				printf("error dequeue at %d\n", i);
				continue;
			}
			printf("p: %p, %p\n", d0, d1);
			printf("value is %d, %d\n", d1->value, d1->other_val);
		}
	}

	queue_destroy(buffer);
	printf("Done!\n");

	return 0;
}