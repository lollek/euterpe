#include "euterpe.h"
/*
  Info here: 
  http://developer.spotify.com/docs/libspotify/12.1.45/structsp__session__callbacks.html
 */

extern int debug;
extern audio_fifo_t g_audio;

void logged_in(sp_session* sess, sp_error err) {

  if (err == SP_ERROR_OK) {
    fprintf(stdout, "Logged on to spotify as user %s\n", sp_session_user_name(sess));

  } else {
    fprintf(stderr, "Failed to log in to spotify: %s\n", sp_error_message(err));
    euterpe_exit(sess);
  }
}

void metadata_updated(sp_session* sess) {
  UNUSED(sess);

  if (debug)
    printf("\033[1;30mMetadata Updated! I could start playing now?\033[0m\n");
  /* FIX
  
  if ((err = sp_track_error(g_track)) != SP_ERROR_OK) {
    fprintf(stderr, "Failed to start playback: %s\n", sp_error_message(err));
    return;
  }
  
  fprintf(stdout, "Now playing: \"%s\"\n", sp_track_name(g_track));
  sp_session_player_load(g_sess, g_track);
  sp_session_player_play(g_sess, 1);
  */
}

void connection_error(sp_session* sess, sp_error err) {
  UNUSED(sess);
  fprintf(stderr, "Could not connect to spotify: %s\n", sp_error_message(err));
}

void message_to_user(sp_session* sess, const char* message) {
  UNUSED(sess);
  fprintf(stdout, "Message to user: %s\n", message);
}


int music_delivery(sp_session* sess, const sp_audioformat* format, 
                   const void* frames, int num_frames) {

  audio_fifo_t *audio_t = &g_audio;
  audio_fifo_data_t *data;
  size_t size;

  UNUSED(sess);

  if (debug)
    fprintf(stdout, "\033[1;30mMusic delivery!\033[0m\n");

  if (num_frames == 0)
    return 0;
   
  pthread_mutex_lock(&audio_t->mutex);
  
  if (audio_t->qlen > format->sample_rate) {
    pthread_mutex_unlock(&audio_t->mutex);
    return 0;
  }
  
  size = num_frames * sizeof(int16_t) * format->channels;
  data = malloc(sizeof(*data) + size);
  memcpy(data->samples, frames, size);

  data->nsamples = num_frames;
  data->rate = format->sample_rate;
  data->channels = format->channels;
  
  TAILQ_INSERT_TAIL(&audio_t->q, data, link);
  audio_t->qlen += num_frames;
  
  pthread_cond_signal(&audio_t->cond);
  pthread_mutex_unlock(&audio_t->mutex);
  
  return num_frames;
}

void play_token_lost(sp_session* sess) {
  UNUSED(sess);
  fprintf(stdout, "Play Token Lost!\n"
          "Your account is being used somewhere else\n");
  /* FIX
     This needs to pause play!
   */
}

void log_message(sp_session* sess, const char* message) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30m%s\033[0m", message);
}

void end_of_track(sp_session* sess) {

  if (debug)
    fprintf(stdout, "\033[1;30mEnd of track!\033[0m\n");
  
  euterpe_play_track(sess, 0, 1);
}

void streaming_error(sp_session* sess, sp_error err) {
  UNUSED(sess);
  fprintf(stderr, "Streaming error occurred: %s\n", sp_error_message(err));
}

void userinfo_updated(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mUserinfo updated!\033[0m\n");
}

void start_playback(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mPlayback started?\033[0m\n");
  /* FIX
     What is this?
   */
}

void stop_playback(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mPlayback stopped?\033[0m\n");
  /* FIX
     What is this?
   */
}

void get_audio_buffer_stats(sp_session* sess, sp_audio_buffer_stats *stats) {
  /* FIX
     What is this?
   */
}

void offline_status_updated(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    printf("\033[1;30mOffline status updated\033[0m\n");
}

void offline_error(sp_session* sess, sp_error err) {
  UNUSED(sess);
  fprintf(stderr, "Offline error: %s\n", sp_error_message(err));
  /* FIX
     What is this?
  */
}

void credentials_blob_updated(sp_session* sess, const char* blob) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mReceived blob: %s\033[0m\n", blob);
  /* OPT
     Do I need to do more than this?
  */
}

void connectionstate_updated(sp_session* sess) {
  UNUSED(sess);
  if (debug)
    fprintf(stdout, "\033[1;30mConnectionstate updated!\033[0m\n");
}

void scrobble_error(sp_session* sess, sp_error err) {
  UNUSED(sess);
  fprintf(stderr, "Scrobble error: %s\n", sp_error_message(err));
}

void private_session_mode_changed(sp_session* sess, bool is_private) {
  UNUSED(sess);
  fprintf(stdout, "Private Session mode changed to %s", is_private? "True":"False");
}
