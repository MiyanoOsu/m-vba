#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#include "sound_output.h"

static snd_pcm_t *handle;

uint32_t Audio_Init()
{
	snd_pcm_hw_params_t *params;
	uint32_t val;
	int32_t dir = -1;
	snd_pcm_uframes_t frames;
	
	/* Open PCM device for playback. */
	int32_t rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
		
	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
	{
		fprintf(stderr, "unable to open PCM device: %s\n", snd_strerror(rc));
		return 1;
	}
	
#ifdef BLOCKING_AUDIO
	snd_pcm_nonblock(handle, 0);
#else
	snd_pcm_nonblock(handle, 1);
#endif

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	rc = snd_pcm_hw_params_any(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_any %s\n", snd_strerror(rc));
		return 1;
	}

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_access %s\n", snd_strerror(rc));
		return 1;
	}

	/* Signed 16-bit little-endian format */
	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_format %s\n", snd_strerror(rc));
		return 1;
	}

	/* Two channels (stereo) */
	rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_channels %s\n", snd_strerror(rc));
		return 1;
	}
	
	val = SOUND_OUTPUT_FREQUENCY;
	rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_rate_near %s\n", snd_strerror(rc));
		return 1;
	}

	/* Set period size to settings.aica.BufferSize frames. */
	frames = SOUND_SAMPLES_SIZE;
	rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}
	frames *= 4;
	rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &frames);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return 1;
	}

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(rc));
		return 1;
	}
	
	return 0;
}

void Audio_Write(int16_t* buffer, uint32_t buffer_size)
{
    if (!handle) return;

    int frames_written, total_frames = buffer_size;
    int16_t* ptr = buffer;

    while (total_frames > 0)
    {
        frames_written = snd_pcm_writei(handle, ptr, total_frames);

        if (frames_written < 0)
        {
            if (frames_written == -EPIPE)  // Underrun
            {
                snd_pcm_prepare(handle);  // Reset the stream
            }
            else if (frames_written == -EAGAIN)  // Try again
            {
                continue;
            }
            else
            {
                fprintf(stderr, "ALSA write error: %s\n", snd_strerror(frames_written));
                break;
            }
        }
        else  // Successful write
        {
            total_frames -= frames_written;
            ptr += frames_written * 2;  // Move buffer pointer forward
        }
    }
}


void Audio_Close()
{
	if (handle)
	{
		snd_pcm_drop(handle);
		snd_pcm_close(handle);
		snd_config_update_free_global();
	}
}
