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

      case 'h': {
        printf("Commands:\n\n"

               "next\t\t - next track\n"
               "prev\t\t - previous track\n\n"

               "d\t\t - toggle debug mode\n"
               "lt\t\t - list tracks\n"
               "ll\t\t - list playlists\n"
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
          euterpe_change_track(sess, 0, 1);
      } break;
      
      case 'p': {
        
        /* pt <number> - play track: */
        if (*(p+1) == 't') {
          if (strlen(cmd) > 3)
            euterpe_change_track(sess, 1, atoi(cmd + 3));
          else
            printf("usage: pt <number>\n");
        }

        /* pl <number> - play playlist: */
        else if (*(p+1) == 'l') {
          if (strlen(cmd) > 3)
            euterpe_play_list(sess, atoi(cmd + 3));
          else
            printf("usage: pl <number>\n");
        }
        
        /* prev - play previous track: */
        else if (!strcmp(cmd, "prev\n"))
          euterpe_change_track(sess, 0, -1);
          
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
  char *blob = NULL;
  sp_session *sess = NULL;
  
  fprintf(stdout, "Euterpe v0.1\n\n");
  
  /* Get argv's: */
  while ((c = getopt(argc, argv, "dl:u:")) != -1) {
    switch (c) {
      case 'd': debug = 1; break;
      case 'u': username = optarg; break;
      default: break;
    }
  }
  
  /* Username handling: */
  if (username == NULL) {
    printf("Username: ");
    fflush(stdout);
    fgets(buf, sizeof(buf), stdin);
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
  if (euterpe_init(&sess, username, password, blob) != 0
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
