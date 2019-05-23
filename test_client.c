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
#define LOW_PASS_500

char* SETTINGS_FILENAME="./lowpass_settings.txt";
int param = 0;
FILE *settings_file;
size_t size = 0;
char *buffer = NULL;

#ifdef LOW_PASS_500
  // change brightness
  #define L_500_coef 63  // size of filter window
  float lp_500[L_500_coef] = {
0.001316 ,
-0.000000 ,
-0.009506 ,
-0.009845 ,
0.000000 ,
0.010602 ,
0.011027 ,
-0.000000 ,
-0.011985 ,
-0.012530 ,
0.000000 ,
0.013783 ,
0.014509 ,
-0.000000 ,
-0.016216 ,
-0.017229 ,
0.000000 ,
0.019690 ,
0.021205 ,
-0.000000 ,
-0.025060 ,
-0.027566 ,
0.000000 ,
0.034458 ,
0.039381 ,
-0.000000 ,
-0.055133 ,
-0.068916 ,
0.000000 ,
0.137832 ,
0.275664 ,
0.333333 ,
0.275664 ,
0.137832 ,
0.000000 ,
-0.068916 ,
-0.055133 ,
-0.000000 ,
0.039381 ,
0.034458 ,
0.000000 ,
-0.027566 ,
-0.025060 ,
-0.000000 ,
0.021205 ,
0.019690 ,
0.000000 ,
-0.017229 ,
-0.016216 ,
-0.000000 ,
0.014509 ,
0.013783 ,
0.000000 ,
-0.012530 ,
-0.011985 ,
-0.000000 ,
0.011027 ,
0.010602 ,
0.000000 ,
-0.009845 ,
-0.009506 ,
-0.000000 ,
0.001316 ,
};


// coefficients
  int g1=63;   // scaling
  int h1=0; // offset
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
low_pass_500(jack_default_audio_sample_t out[1],jack_nframes_t delay_index)
{
	
	int i;
	jack_default_audio_sample_t sum;
	jack_nframes_t tmp = delay_index ;
	
	for (i=0; i < L_500_coef; i++)  // loop over all samples of audio stream
	{
		sum += lp_500[i]*delay_line[delay_index];
		delay_index = (delay_index - 1) % latency;		
	}
	sum = sum/g1 + h1;  // scaling and offset

	if (sum < MIN_VALUE) sum = MIN_VALUE; //clipping
	else if (sum > MAX_VALUE) sum=MAX_VALUE;  
			
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
 
	in = jack_port_get_buffer (input_port, nframes);	
	out = jack_port_get_buffer (output_port, nframes);
	
	for (k=0; k<nframes; k++) { 		
		delay_line[delay_index] = in[k];
		
		low_pass_500(result, delay_index);
			
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
	const char *client_name = "Lowpass";
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

