/** @file latent_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <jack/jack.h>
  
time_t last_time=0;

#define MAX_VALUE 1.000000
#define MIN_VALUE -1.000000

// define FIR filter settings (select one)
#define HIGH_PASS_2k
#define HIGH_PASS_4k
#define HIGH_PASS_8k

char* SETTINGS_FILENAME="/home/pi/Qt_Projekt/test/highpass_settings.txt";
int param = 0;
FILE *settings_file;
size_t size = 0;
char *buffer = NULL;

#ifdef HIGH_PASS_2k
  // change brightness
  #define H_2k_coef 49   // size of filter window
  int c_2k[H_2k_coef] = {
  11, -7, -5, -3, -2, -1, 0, 1, 2, 2, 3, 3, 3, 2, 1, 0, -2, -4, -6, -8, -10, -12, -13, -14, 114,
  -14, -13, -12, -10, -8, -6, -4, -2, 0, 1, 2, 3, 3, 3, 2, 2, 1, 0, -1, -2, -3, -5, -7, 11 }; // coefficients
  int g1=49;   // scaling
  int h1=0; // offset
#endif	

#ifdef HIGH_PASS_4k
  // change brightness
  #define H_4k_coef 49  // size of filter window
  int c_4k[H_4k_coef] = {
  8, -8, -4, -1, 1, 2, 3, 2, 1, -1, -2, -3, -3, -1, 1, 3, 5, 5, 3, -1, -7, -13, -19, -23, 103, -23, -19, -13, -7,
  -1, 3, 5, 5, 3, 1, -1, -3, -3, -2, -1, 1, 2, 3, 2, 1, -1, -4, -8, 8 }; // coefficients
  int g2=49;   // scaling
  int h2=0; // offset
#endif	

#ifdef HIGH_PASS_8k
  // change brightness
  #define H_8k_coef 49  // size of filter window
  int c_8k[H_8k_coef] = {
  3, 0, -8, 8, 0, -1, -3, -1, 2, 3, 0, -3, -3, 0, 4, 3, -2, -6, -3, 5, 10, 3, -16, -37, 82, -37, -16,
  3, 10, 5, -3, -6, -2, 3, 4, 0, -3, -3, 0, 3, 2, -1, -3, -1, 0, 8, -8, 0, 3 }; // coefficients
  int g3=49;   // scaling
  int h3=0; // offset
#endif

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

jack_default_audio_sample_t *delay_line;
jack_nframes_t delay_index;
jack_nframes_t latency = 1024;

#ifdef WIN32
#define jack_sleep(val) Sleep((val))
#else
#define jack_sleep(val) usleep((val) * 1000)
#endif


void
high_pass_2k(jack_default_audio_sample_t out[1],jack_nframes_t delay_index)
{
	
	int i;
	jack_default_audio_sample_t sum;
	jack_nframes_t tmp = delay_index ;
	
	for (i=0; i < H_2k_coef; i++)  // loop over all samples of audio stream
	{
		sum += c_2k[i]*delay_line[delay_index];
		delay_index = (delay_index - 1) % latency;		
	}
	sum = sum/g1 + h1;  // scaling and offset

	if (sum < MIN_VALUE) sum = MIN_VALUE; //clipping
	else if (sum > MAX_VALUE) sum=MAX_VALUE;  
			
	delay_index = tmp;
	out[0] = sum;
}

void
high_pass_4k(jack_default_audio_sample_t out[1],jack_nframes_t delay_index)
{
	int i;
	jack_default_audio_sample_t sum;
	jack_nframes_t tmp = delay_index ;
	
	for (i=0; i < H_4k_coef; i++)  // loop over all samples of audio stream
	{
		sum += c_4k[i]*delay_line[delay_index];
		delay_index = (delay_index - 1) % latency; 		
	}
	sum = sum/g2 + h2;  // scaling and offset

	if (sum < MIN_VALUE) sum = MIN_VALUE; //clipping
	else if (sum > MAX_VALUE) sum=MAX_VALUE;  
			
	// write to output
	delay_index = tmp;
	out[0] = sum;
}

void
high_pass_8k(jack_default_audio_sample_t out[1],jack_nframes_t delay_index)
{
	int i;
	jack_default_audio_sample_t sum;
	jack_nframes_t tmp = delay_index ;
	
	for (i=0; i < H_8k_coef; i++)  // loop over all samples of audio stream
	{
		sum += c_8k[i]*delay_line[delay_index];
		delay_index = (delay_index - 1) % latency; 		
	}
	sum = sum/g3 + h3;  // scaling and offset

	if (sum < MIN_VALUE) sum = MIN_VALUE; //clipping
	else if (sum > MAX_VALUE) sum=MAX_VALUE;  
			
	// write to output
	delay_index = tmp;
	out[0] = sum;
}

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */

int
process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in, *out;
	jack_default_audio_sample_t result[1];
	int k;
	
	settings_file = fopen(SETTINGS_FILENAME, "r");  // open raw file
	if (settings_file == NULL)                     // check if opening worked fine
	{
		printf("Error opening file  ==> exit.\n");
		exit(-1);
	}	
	getline(&buffer,&size,settings_file);               // read first line from settings file
	sscanf(buffer, "%d", &param);                      // convert the line to an integer value of the first parameter
	fclose(settings_file);
 
	in = jack_port_get_buffer (input_port, nframes);	
	out = jack_port_get_buffer (output_port, nframes);
	
	for (k=0; k<nframes; k++) { 		
		delay_line[delay_index] = in[k];
		if(param == 1)
		{
			high_pass_2k(result, delay_index);
		}
		else if(param == 2)
		{
			high_pass_4k(result, delay_index);
		}
		else if(param == 3)
		{
			high_pass_8k(result, delay_index);
		}
			
		out[k] = result[0];			
		delay_index = (delay_index + 1) % latency;
	}
	
	//reconstruct_array(tmp, delay_index);
	return 0;
}

void
latency_cb (jack_latency_callback_mode_t mode, void *arg)
{
	jack_latency_range_t range;
	if (mode == JackCaptureLatency) {
		jack_port_get_latency_range (input_port, mode, &range);
		range.min += latency;
		range.max += latency;
		jack_port_set_latency_range (output_port, mode, &range);
	} else {
		jack_port_get_latency_range (output_port, mode, &range);
		range.min += latency;
		range.max += latency;
		jack_port_set_latency_range (input_port, mode, &range);
	}
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
    fprintf(stderr, "JACK shut down, exiting ...\n");
	exit (1);
}

int
main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = "Highpass";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;


	if (argc == 2)
		latency = atoi(argv[1]);

	delay_line = malloc( latency * sizeof(jack_default_audio_sample_t));
	if (delay_line == NULL) {
		fprintf (stderr, "no memory");
		exit(1);
	}

	memset (delay_line, 0, latency * sizeof(jack_default_audio_sample_t));

	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `latency()' whenever
	   the latency needs to be recalculated.
	*/
	if (jack_set_latency_callback)
		jack_set_latency_callback (client, latency_cb, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate.
	 */

	printf ("engine sample rate: %" PRIu32 "\n",
		jack_get_sample_rate (client));

	/* create two ports */

	input_port = jack_port_register (client, "input",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsInput, 0);
	output_port = jack_port_register (client, "output",
					  JACK_DEFAULT_AUDIO_TYPE,
					  JackPortIsOutput, 0);

	if ((input_port == NULL) || (output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		exit (1);
	}

	if (jack_connect (client, ports[0], jack_port_name (input_port))) {
		fprintf (stderr, "cannot connect input ports\n");
	}

	free (ports);

	ports = jack_get_ports (client, NULL, NULL,
				JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

	if (jack_connect (client, jack_port_name (output_port), ports[0])) {
		fprintf (stderr, "cannot connect output ports\n");
	}

	free (ports);

	/* keep running until stopped by the user */

	jack_sleep (-1);

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (client);
	exit (0);
}

