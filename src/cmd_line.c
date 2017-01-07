#define CMD_LINE_C
/*
 *  Copyright (C) 1999 James Antill
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * email: james@twocan.org
 */
#include "main.h"


void init_cmd_line(int argc, char *argv[])
{ /* NOTE: if you change/add then you need to do the angel too */
  char optchar = 0;
  const char *program_name = "talker";
  struct option long_options[] =
  {
   {"configure", required_argument, NULL, 'C'},
   {"help", no_argument, NULL, 'h'},
   {"port", required_argument, NULL, 'p'},
   {"read-only", optional_argument, NULL, 'R'},
   {"root-dir", required_argument, NULL, 'r'},
   {"verbose", no_argument, NULL, 'd'},   
   {"version", no_argument, NULL, 'V'},   
   {NULL, 0, NULL, 0}
  };

  if (argv[0])
  {
   if ((program_name = C_strrchr(argv[0], '/')))
     ++program_name;
   else
     program_name = argv[0];
  }
  
  while ((optchar = getopt_long(argc, argv, "c:C:dhp:r:vHR:V",
                                long_options, NULL)) != EOF)
    switch (optchar)
    {
     case '?':
       fprintf(stderr, " That option is not valid.\n");
     case 'H':
     case 'h':
       printf("\n Format: %s [-dhpvHPV]\n"
              " --help -h         - Print this message.\n"
              " --port -p         - Change the port number.\n"
              " --read-only -R    - Make program never do write operations.\n"
              " --root-dir -r     - Change the root dir.\n"
              " --verbose -d      - Print some debuging stuff.\n"
              " --version -v      - Print the version string.\n",
              program_name);
       if (optchar == '?')
         exit (EXIT_FAILURE);
       else
         exit (EXIT_SUCCESS);
       
     case 'v':
     case 'V':
       printf(" %s is version %s, package version %s.\n",
              program_name, VERSION, TALKER_CODE_SNAPSHOT);
       exit (EXIT_SUCCESS);

     case 'p':
       if (!configure_add_interface(atoi(optarg),
                                    CONFIGURE_INTERFACE_TYPE_ANY))
         SHUTDOWN_MEM_ERR();
       break;

     case 'R': /* of course ... this will never save as true */
       if (optarg && TOGGLE_MATCH_ON(optarg))
         configure.talker_read_only = TRUE;
       else if (optarg && TOGGLE_MATCH_OFF(optarg))
         configure.talker_read_only = FALSE;
       else if (!optarg || TOGGLE_MATCH_TOGGLE(optarg))
         configure.talker_read_only = !configure.talker_read_only;
       else
       {
        printf(" %s --read-only[=on|off|toggle]\n", program_name);
        exit (EXIT_FAILURE);
       }
       break;
       
     case 'r':
       root_dir = optarg;
       break;
       
     case 'd':
       configure.talker_verbose = TRUE;
       break;

     case 'c':
     case 'C':
       COPY_STR(configure.configure_file_name, optarg, CONFIGURE_FILE_NAME_SZ);
       break;
    }
}
