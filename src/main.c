#include "euterpe.h"

extern int debug;

void handle_input(sp_session *sess) {

  char cmd[256], *p;
  
  for (;;) {
    
    printf("\n> ");
    fgets(cmd, sizeof(cmd), stdin);
    p = cmd;
    
    switch(*p) {

      case 'd': {
        printf("Debugmode changed to %s\n", debug? "off":"on");
        debug = !debug;
      } break;

      case 'f': {
        if (strlen(cmd) == 13 && debug && !strcmp(cmd, "force-update\n")) {
          euterpe_loop_activate(sess);
          printf("Forced update on euterpe-loop\n");
        }
        else
          printf("Unknown command. Type h for help\n");
      } break;

      case 'h': {
        printf("Commands:\n\n"

               "next\t\t - next track\n"
               "prev\t\t - previous track\n\n"

               "d\t\t - toggle debug mode\n"
               "lt\t\t - list tracks\n"
               "ll\t\t - list playlists\n"
               "p\t\t - toggle play/pause"
               "pt <number>\t - play track\n"
               "pl <number>\t - play playlist\n"
               "q\t\t - quit\n");
      } break;
        
      case 'l': {

        /* lt - list tracks: */
        if (*(p+1) == 't') {
          if (strlen(cmd) > 3)
            euterpe_display_tracks(sess, atoi(cmd + 3));
          else
            euterpe_display_tracks(sess, -1);
        }

        /* ll - list playlists: */
        else if (*(p+1) == 'l')
          euterpe_display_playlists(sess);

        else
          printf("Unknown command. Type h for help\n");
      } break;

      case 'n': {
        if (!strcmp(cmd, "next\n"))
          euterpe_play_track(sess, 0, 1);
      } break;
      
      case 'p': {

        /* p - toggle pause/play: */
        if (*(p+1) == '\n')
          euterpe_play_pause_toggle(sess);

        /* pt <number> - play track: */
        else if (*(p+1) == 't') {
          if (strlen(cmd) > 4)
            euterpe_play_track(sess, 1, atoi(cmd + 3));
          else
            printf("usage: pt <number>\n");
        }

        /* pl <number> - play playlist: */
        else if (*(p+1) == 'l') {
          if (strlen(cmd) > 4)
            euterpe_set_playlist(sess, atoi(cmd + 3));
          else
            printf("usage: pl <number>\n");
        }
        
        /* prev - play previous track: */
        else if (!strcmp(cmd, "prev\n"))
          euterpe_play_track(sess, 0, -1);
          
        else
          printf("Unknown command. Type h for help\n");
      } break;

      case 'q': { 
        euterpe_exit(sess); 
      } break;

      default: 
        printf("Unknown command. Type h for help\n");
        break;
    }
  }
}

int main(int argc, char** argv) {

  char c, buf[256];
  char *username = NULL;
  char *password = NULL;
  sp_session *sess = NULL;
  
  fprintf(stdout, "Euterpe v0.1.3\n\n");
  
  /* Get argv's: */
  while ((c = getopt(argc, argv, "du:")) != -1) {
    switch (c) {
      case 'd': debug = 1; break;
      case 'u': username = optarg; break;
      default: {
        printf("\n"
               "Usage: %s <options>\n"
               "Options:\n"
               "-d\t\t - Start with debug-mode on\n"
               "-u <username>\t - login with <username>\n",
               argv[0]);
        exit(1);
      } break;
    }
  }
  
  /* Username handling: */
  if (username == NULL) {
    printf("Username: ");
    fflush(stdout);
    
    fgets(buf, sizeof(buf), stdin);
    for(username = buf; *username != '\0'; username++)
      if (*username == '\n')
        *username = '\0';
    username = buf;
  }
  if (strlen(username) <= 0) {
    printf("Blank username received\n");
    return 1;
  }

  /* Password Handling: */
  password = getpass("Password: ");
  if (strlen(password) <= 0) {
    printf("Blank password received\n");
    return 1;
  }
  
  /* Create session: */
  if (euterpe_init(&sess, username, password) != 0
      || sess == NULL) {
    printf("Failed to create a spotify session\n");
    return 1;
  }
  free(password);
  
  /* Main: */
  handle_input(sess);
  euterpe_exit(sess);

  return 0;
}
