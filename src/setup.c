/* setup -- meta configuration tool calls other configuration tools.
 * Copyright (C) 1999-2002,2004-2007 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <dirent.h>
#include <libintl.h>
#include <limits.h>
#include <locale.h>
#include <newt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _(String) gettext(String)

struct tool {
   char *name;
   char *path;
   char *namedomain;
   char *namepath;
   char *file;
};

static int
comparetools(const void *t1, const void *t2)
{
   const struct tool *tool1, *tool2;
   int i;
   tool1 = t1;
   tool2 = t2;
   i = strcoll(tool1->name, tool2->name);
   if (i != 0) {
      return i;
   }
   return strcoll(tool1->file, tool2->file);
}

int
main(int argc, char **argv)
{
   newtComponent lbox, run, quit, form,answer;
   newtGrid grid,grid2;
   long i, command;
   int rc, j;
   char roottext[81], *p, *q, c;
   struct tool *tools;
   ssize_t n_tools;
   DIR *setuptoold;
   FILE *setuptoolf;
   char setuptools[LINE_MAX];
   const char *setupdirs[] = {SETUPTOOLDDIR, OTHERSETUPTOOLDDIR};
   unsigned int setupdir;
   struct dirent *dent;
   char *local;

   local=setlocale(LC_ALL, "");
   /* Terminal can't display Indic languages, so use english instead
    * if working in /dev/ttyX (/dev/pts/X works) */
   if (local && !strncmp(local+2, "_IN", 3)) {
     char *tname=ttyname(1);
     if (tname && !strncmp(tname, "/dev/tty", 8))
     {
       setenv("LC_MESSAGES", "en_US", 1);
       setlocale(LC_ALL, "");
     }
   }
   bindtextdomain(GETTEXT_PACKAGE, "/usr/share/locale");
   textdomain(GETTEXT_PACKAGE);
   
   if (getuid() != 0) {
      fprintf(stderr,
	      _("\nERROR - You must be root to run setup.\n"));
      exit(1);
   }

   tools = NULL;
   n_tools = 0;
   for (setupdir = 0;
	setupdir < sizeof(setupdirs) / sizeof(setupdirs[0]);
	setupdir++) {
      if ((setuptoold = opendir(setupdirs[setupdir])) == NULL) {
#ifdef DEBUG
        fprintf(stderr, "Error scanning \"%s\"!\n", setupdirs[setupdir]);
#endif
	continue;
      }
#ifdef DEBUG
      fprintf(stderr, "Scanning \"%s\"...\n", setupdirs[setupdir]);
#endif
      while ((dent = readdir(setuptoold)) != NULL) {
	 snprintf(setuptools, sizeof(setuptools), "%s/%.*s",
		  setupdirs[setupdir], NAME_MAX, dent->d_name);
         if ((strstr(setuptools, "/.") == NULL) &&
             (strstr(setuptools, ".rpmorig") == NULL) &&
             (strstr(setuptools, ".rpmsave") == NULL) &&
             (strstr(setuptools, ".rpmnew") == NULL) &&
             (strpbrk(setuptools, "!~#") == NULL)) {
	    if ((setuptoolf = fopen(setuptools, "r")) != NULL) {
	       while (fgets(setuptools, sizeof(setuptools),
	                    setuptoolf) != NULL) {
                  p = strpbrk(setuptools, "\r\n#");
		  if (p != NULL) {
		     *p = '\0';
		  }

		  p = strtok(setuptools, "|");
		  if ((p == NULL) || (strlen(p) == 0)) {
		     continue;
		  }
		  if (access(p, X_OK) != 0) {
		     q = strpbrk(p, " \t");
		     if ((q != NULL) && (q > p)) {
		     	c = *q;
			*q = '\0';
		        if (access(p, X_OK) != 0) {
		           continue;
			}
			*q = c;
		     } else {
		        continue;
		     }
		  }

                  tools = realloc(tools, sizeof(struct tool) * (n_tools + 1));
		  if (tools == NULL) {
		     continue;
		  }

		  memset(&tools[n_tools], 0, sizeof(tools[n_tools]));
		  tools[n_tools].path = strdup(p);
		  tools[n_tools].name = tools[n_tools].path;
		  tools[n_tools].namedomain = GETTEXT_PACKAGE;
		  tools[n_tools].namepath = "/usr/share/locale";
		  tools[n_tools].file = strdup(dent->d_name);

		  p = strtok(NULL, "|");
		  if (p == NULL) {
		     n_tools++;
		     continue;
		  }
		  tools[n_tools].name = strdup(p);

		  p = strtok(NULL, "|");
		  if (p == NULL) {
		     n_tools++;
		     continue;
		  }
		  if (strlen(p) > 0) {
		     tools[n_tools].namedomain = strdup(p);
		  }

		  p = strtok(NULL, "|");
		  if (p == NULL) {
		     n_tools++;
		     continue;
		  }
		  if (strlen(p) > 0) {
		     tools[n_tools].namepath = strdup(p);
		  }

		  n_tools++;
	       }
	       fclose(setuptoolf);
	    }
	 }
      }
      closedir(setuptoold);
   }
   if (n_tools == 0) {
      fprintf(stderr,
	      _("\nERROR - No tool descriptions found in %s or %s.\n"),
	      SETUPTOOLDDIR, OTHERSETUPTOOLDDIR);
      exit(1);
   }
   for (i = 0; i < n_tools; i++) {
      bindtextdomain(tools[i].namedomain, tools[i].namepath);
      tools[i].name = dgettext(tools[i].namedomain, tools[i].name);
   }
   qsort(tools, n_tools, sizeof(tools[0]), comparetools);
   for (i = 0; i < n_tools - 1; i++) {
      while ((i < n_tools - 1) &&
	     (strcmp(tools[i].name, tools[i + 1].name) == 0)) {
#ifdef DEBUG
         fprintf(stderr, "Removing duplicate %s = %s (%s)\n", tools[i + 1].name,
	 	 tools[i + 1].path, tools[i + 1].file);
#endif
         memmove(&tools[i + 1], &tools[i + 2],
		 sizeof(tools[i + 2]) * (n_tools - (i + 2)));
	 n_tools--;
      }
   }

   lbox = newtListbox(2, 1, n_tools, NEWT_FLAG_RETURNEXIT);
   for (i = 0; i < n_tools; i++) {
      snprintf(setuptools, sizeof(setuptools), "   %s   ", tools[i].name);
      while ((setuptools[1] != '\0') &&
	     (isspace((unsigned char)setuptools[1]))) {
         memmove(setuptools + 1, setuptools + 2, strlen(setuptools + 1));
      }
      j = strlen(setuptools);
      while ((j > 2) && isspace((unsigned char)setuptools[j - 2])) {
      	 setuptools[j - 1] = '\0';
	 j--;
      }
      newtListboxAddEntry(lbox, setuptools, (void*) i);
#ifdef DEBUG
      fprintf(stderr, "%s = %s\n", setuptools, tools[i].path);
#endif
   }
   newtInit();
   newtCls();
   newtPushHelpLine(_("    <Tab>/<Alt-Tab> between elements  |"
		      "    Use <Enter> to edit a selection") );
   snprintf(roottext, sizeof(roottext),
	    _("Text Mode Setup Utility %-28s (c) 1999-2006 Red Hat, Inc."),
	    VERSION);
   newtDrawRootText(0, 0, roottext);
   form = newtForm(NULL, NULL, 0);
   grid = newtCreateGrid(1, 2);
   grid2 = newtButtonBar(_("Run Tool"), &run, _("Quit"), &quit, NULL);
   newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, lbox,
		    1, 0, 1, 0, NEWT_ANCHOR_TOP, 0);
   newtGridSetField(grid, 0, 1, NEWT_GRID_SUBGRID, grid2,
		    0, 1, 0, 0, NEWT_ANCHOR_TOP, 0);
   newtGridWrappedWindow(grid,_("Choose a Tool"));
   newtFormAddComponents(form, lbox, run, quit, NULL);
   do {
      answer = newtRunForm(form);
      if (answer != quit) {
	 command = (long) newtListboxGetCurrent(lbox);
	 rc = 0;
      } else {
	 rc = -1;
      }
      if (rc != -1) {
	 newtSuspend();
#ifdef DEBUG
         fprintf(stderr, "Running \"%s\".\n", tools[command].path);
#endif
	 system(tools[command].path);
	 newtResume();
      }
   } while (rc != -1);
   newtPopWindow();
   newtFormDestroy(form);
   newtFinished();
   return 0;
}
