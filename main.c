#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

typedef struct loadedmodule_t {
	void *module_handle;

	int (*module_init)();
	int (*module_run)();
	void (*module_deinit)();
} loadedmodule_t;

void print_progress_bar(int current, int total) {
	const int bar_width = 50;

	// Calculate the ratio of current progress to total
	float ratio = (float)current / total;

	// Determine how many characters to fill in the progress bar
	int filled = (int)(bar_width * ratio);

	// Print the progress bar
	printf("|");
	for(int i = 0; i < bar_width; ++i) {
		if(i < filled)
			printf("=");
		else if (i == filled)
			printf(">");
		else
			printf(" ");
	}
	printf("| %3d%%\r", (int)(ratio * 100));
}



int main(int argc, char **argv) {
	size_t module_count = argc - 1;
	if (module_count == 0) return -1;

	loadedmodule_t *modules = calloc(module_count, sizeof(loadedmodule_t));
	bool is_running = true;

	printf("Linking modules...\n");
	for (int i = 1; i < argc; ++i) {
		void *handle = dlopen(argv[i], RTLD_LAZY | RTLD_GLOBAL);
		if (!handle) {
			fprintf(stderr, "%s\n", dlerror());
			exit(EXIT_FAILURE);
		}

		modules[i - 1].module_handle = handle;

		dlerror();    /* Clear any existing error */

		print_progress_bar(i, module_count);

		int (*module_init)() = (int (*)())dlsym(handle, "module_init");
		char *error = dlerror();
		if (error != NULL) {
			fprintf(stderr, "\n%s\n", error);
			exit(EXIT_FAILURE);
		}

		int (*module_run)() = (int (*)())dlsym(handle, "module_run");
		error = dlerror();
		if (error != NULL) {
			fprintf(stderr, "\n%s\n", error);
			exit(EXIT_FAILURE);
		}

		void (*module_deinit)() = (void (*)())dlsym(handle, "module_deinit");
		error = dlerror();
		if (error != NULL) {
			fprintf(stderr, "\n%s\n", error);
			exit(EXIT_FAILURE);
		}



		modules[i - 1].module_init = module_init;
		modules[i - 1].module_run = module_run;
		modules[i - 1].module_deinit = module_deinit;

	}

	printf("\n");

	printf("Initializing modules...\n");
	for (int i = 0; i < module_count; ++i) {
		modules[i].module_init();
	}

	while(is_running) {
		for (int i = 0; i < module_count; ++i) {
			if(modules[i].module_run() == -1) {
				is_running = false;
			}
		}
	}

	for (int i = module_count - 1; i >= 0; --i) {
		modules[i].module_deinit();
		dlclose(modules[i].module_handle);
	}

	free(modules);

	return EXIT_SUCCESS;
}
