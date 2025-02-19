/*
 * Module to simplify drawing an interactive menu a la curses and 
 * ncurses. The module detects the amount of space available in the menu,
 * creates a grid of characters, and supports writing to this grid.
 *
 * Functions for auto-wrapping text are included.
 *
 * Also supported are background boxes, to mark certain selections.
 *
 * Copyright (C) 2025 K. Urbański <karol.jakub.urbanski@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <string.h>
#include "quakedef.h"
#include "qcurses.h"
#include "ghost/demosummary.h"
#include "ghost/ghost.h"

qcurses_box_t * main_box = NULL;
qcurses_box_t * local_box = NULL;
static enum demos_tabs demos_tab = TAB_LOCAL_DEMOS;
static qboolean demos_update = true;

extern direntry_t * filelist;
extern int num_files;
extern char *GetPrintedTime(double time);

qcurses_char_t * news = NULL;

static char *toYellow (char *s)
{
	static	char	buf[20];

	Q_strncpyz (buf, s, sizeof(buf));
	for (s = buf ; *s ; s++)
		if (*s >= '0' && *s <= '9')
			*s = *s - '0' + 18;

	return buf;
}

qcurses_box_t * qcurses_init(int cols, int rows) {
    qcurses_box_t * box = malloc(sizeof(qcurses_box_t));

    box->cols = cols;
    box->rows = rows;

    box->grid = calloc(rows, sizeof(qcurses_char_t *));
    for (int i = 0; i < rows; i++){
        box->grid[i] = calloc(cols, sizeof(qcurses_char_t));
        for (int j = 0; j < cols; j++)
            box->grid[i][j].symbol = ' ';
    }

    return box;
}

void qcurses_free(qcurses_box_t *box){
    if (!box)
        return;

    for (int i = 0; i < box->rows; i++)
        free(box->grid[i]);
    free(box->grid);
    free(box);
    box = NULL;
}

void qcurses_display(qcurses_box_t *src){
    int i, j;
    for (i = 0; i < src->rows; i++) {
        for (j = 0; j < src->cols; j++) {
            Draw_Character(j * 8, i * 8, src->grid[i][j].symbol, false);
        }
    }
}

void qcurses_boxprint_wrapped(qcurses_box_t *dest, qcurses_char_t *src, size_t size){
    int row = 0, col = 0;
    for(int i = 0; i < size && (src + i)->symbol; i++){
        if ((src + i)->symbol == '\n' || col >= dest->cols){
            col = 0;
            row++;
        }
        if ((src + i)->symbol != '\n')
            memcpy(dest->grid[row] + col++, src + i, sizeof(qcurses_char_t));
    }
}

void qcurses_print(qcurses_box_t *dest, int col, int row, char *src, qboolean bold){
    if (row >= dest->rows)
        return;

    for(int i = 0; i < strlen(src) && i + col < dest->cols; i++)
        dest->grid[row][col + i].symbol = src[i] | (bold ? 128 : 0);
}

void qcurses_insert(qcurses_box_t *dest, int col, int row, qcurses_box_t *src) {
    for (int i = row; i < row + src->rows && i < dest->rows; i++){
        memcpy(dest->grid[i] + col, src->grid[i - row], sizeof(qcurses_char_t) * min(src->cols, dest->cols - col));
    }
}

void M_Demos_KeyHandle_Local (int k) {

}

void M_Demos_KeyHandle (int k)
{
    demos_update = true;

    switch (k) {
    case K_ESCAPE:
    case K_MOUSE2:
        M_Menu_Main_f ();
        break;
    case 'h':
    case K_LEFTARROW:
        demos_tab = max(TAB_LOCAL_DEMOS, demos_tab - 1);
        S_LocalSound("misc/menu1.wav");
        break;
    case 'l':
    case K_RIGHTARROW:
        demos_tab = min(TAB_SDA_DATABASE, demos_tab + 1);
        S_LocalSound("misc/menu1.wav");
        break;
    }

    switch (demos_tab) {
    case TAB_LOCAL_DEMOS:
        M_Demos_KeyHandle_Local(k);
        break;
    default:
        break;
    }

        //M_List_Key (k, num_files, MAXLINES);

        //switch (k)
        //{
        //case K_ESCAPE:
        //case K_MOUSE2:
        //	if (searchbox)
        //	{
        //		KillSearchBox ();
        //	}
        //	else
        //	{
        //		Q_strncpyz (prevdir, filelist[list_base+list_cursor].name, sizeof(prevdir));
        //		M_Menu_Main_f ();
        //	}
        //	break;

        //case K_ENTER:
        //case K_MOUSE1:
        //	if (!num_files || filelist[list_base+list_cursor].type == 3)
        //		break;

        //	if (keydown[K_CTRL] && keydown[K_SHIFT])
        //	{
        //		Cbuf_AddText ("ghost_remove\n");
        //	}
        //	else if (filelist[list_cursor+list_base].type)
        //	{
        //		if (filelist[list_base+list_cursor].type == 2)
        //		{
        //			char	*p;

        //			if ((p = strrchr(demodir, '/')))
        //			{
        //				Q_strncpyz (prevdir, p + 1, sizeof(prevdir));
        //				*p = 0;
        //			}
        //		}
        //		else
        //		{
        //			strncat (demodir, va("/%s", filelist[list_base+list_cursor].name), sizeof(demodir) - 1);
        //		}
        //		SearchForDemos ();
        //	}
        //	else
        //	{
        //		if (keydown[K_CTRL] && !keydown[K_SHIFT])
        //		{
        //			Cbuf_AddText (va("ghost \"..%s/%s\"\n", demodir, filelist[list_base+list_cursor].name));
        //		}
        //		else
        //		{
        //			key_dest = key_game;
        //			m_state = m_none;
        //			Cbuf_AddText (va("playdemo \"..%s/%s\"\n", demodir, filelist[list_base+list_cursor].name));
        //		}
        //		Q_strncpyz (prevdir, filelist[list_base+list_cursor].name, sizeof(prevdir));
        //	}

        //	if (searchbox)
        //		KillSearchBox ();
        //	break;

        //case K_BACKSPACE:
        //	if (strcmp(searchfile, ""))
        //		searchfile[--num_searchs] = 0;
        //	break;

        //default:
        //	if (k < 32 || k > 127)
        //		break;

        //	searchbox = true;
        //	searchfile[num_searchs++] = k;
        //	worx = false;
        //	for (i=0 ; i<num_files ; i++)
        //	{
        //		if (strstr(filelist[i].name, searchfile) == filelist[i].name)
        //		{
        //			worx = true;
        //			S_LocalSound ("misc/menu1.wav");
        //			list_base = i - 10;
        //			if (list_base < 0 || num_files < MAXLINES)
        //			{
        //				list_base = 0;
        //				list_cursor = i;
        //			}
        //			else if (list_base > (num_files - MAXLINES))
        //			{
        //				list_base = num_files - MAXLINES;
        //				list_cursor = MAXLINES - (num_files - i);
        //			}
        //			else
        //				list_cursor = 10;
        //			break;
        //		}
        //	}
        //	if (!worx)
        //		searchfile[--num_searchs] = 0;
        //	break;
        //}
}

void qcurses_make_bar(qcurses_box_t * box, int row){
    for (int i = 0; i < box->cols; i++)
        qcurses_print(box, i, row, "\x1e", true);
    qcurses_print(box, 0, row, "\x1d", true);
    qcurses_print(box, box->cols - 1, row, "\x1f", true);
}

void M_Demos_DisplayLocal (int cols, int rows, int start_col, int start_row) {
    qcurses_box_t * local_box  = qcurses_init(cols, rows);
    qcurses_box_t * map_box    = qcurses_init(25, rows);
    qcurses_box_t * skill_box  = qcurses_init(10, rows);
    qcurses_box_t * kill_box   = qcurses_init(12, rows);
    qcurses_box_t * secret_box = qcurses_init(9, rows);
    qcurses_box_t * time_box   = qcurses_init(15, rows);
    qcurses_box_t * player_box = qcurses_init(15, rows);
    qcurses_box_t * size_box   = qcurses_init(8, rows);
    qcurses_box_t * name_box   = qcurses_init(cols - map_box->cols - skill_box->cols - kill_box->cols - secret_box->cols - time_box->cols - size_box->cols - player_box->cols, rows);

    /* header */
    qcurses_print(name_box, 0, 0, "NAME", true);
    qcurses_make_bar(name_box, 1);
    qcurses_print(map_box, 0, 0, "MAP", true);
    qcurses_make_bar(map_box, 1);
    qcurses_print(player_box, 0, 0, "PLAYER", true);
    qcurses_make_bar(player_box, 1);
    qcurses_print(skill_box, 0, 0, "SKILL", true);
    qcurses_make_bar(skill_box, 1);
    qcurses_print(kill_box, 0, 0, "KILLS", true);
    qcurses_make_bar(kill_box, 1);
    qcurses_print(secret_box, 0, 0, "SECRETS", true);
    qcurses_make_bar(secret_box, 1);
    qcurses_print(time_box, 0, 0, "TIME", true);
    qcurses_make_bar(time_box, 1);
    qcurses_print(size_box, 0, 0, "SIZE", true);
    qcurses_make_bar(size_box, 1);

    for (int i = 0; i < num_files && i < local_box->rows - 2; i++){
        direntry_t * d = filelist + i;
        char str[100];
        Q_strncpyz (str, d->name, sizeof(str));
        qcurses_print(name_box, 0, i + 2, str, !d->type);

        switch (d->type) {
        case 0:
            qcurses_print(size_box, 0, i + 2, toYellow(va("%7ik", (d->size) >> 10)), false); 
            break;
        case 1:
            qcurses_print(size_box, 0, i + 2, "  folder", false); 
            break;
        case 2: 
            qcurses_print(size_box, 0, i + 2, "      up", false); 
            break;
        }

        if (d->type == 0) {
            FILE *demo_file = NULL;
            char demo_path[MAX_OSPATH];
            Q_strlcpy(demo_path, d->name, sizeof(demo_path));
            demo_summary_t demo_summary;
            demo_file = Ghost_OpenDemoOrDzip(demo_path);

            if (demo_file && DS_GetDemoSummary(demo_file, &demo_summary)) {
                if (demo_summary.skill == 3)
                    qcurses_print(skill_box, 0, i + 2, "Nightmare", false); 
                else if (demo_summary.skill == 0)
                    qcurses_print(skill_box, 0, i + 2, "Easy", false); 

                if (demo_summary.num_maps == 1 || !demo_summary.total_time)
                    Q_snprintfz(str, sizeof(str), "%s", demo_summary.maps[0]);
                else
                    Q_snprintfz(str, sizeof(str), "%s +", demo_summary.maps[0]);
                qcurses_print(map_box, 0, i + 2, str, false); 

                Q_snprintfz(str, sizeof(str), "%s", demo_summary.client_names[0]);
                qcurses_print(player_box, 0, i + 2, str, true); 

                Q_snprintfz(str, sizeof(str), "%4d/", demo_summary.kills);
                qcurses_print(kill_box, 0, i + 2, str, true); 

                Q_snprintfz(str, sizeof(str), "%4d", demo_summary.total_kills);
                qcurses_print(kill_box, 5, i + 2, toYellow(str), true); 

                Q_snprintfz(str, sizeof(str), "%3d/", demo_summary.secrets);
                qcurses_print(secret_box, 0, i + 2, str, false); 

                Q_snprintfz(str, sizeof(str), "%3d", demo_summary.total_secrets);
                qcurses_print(secret_box, 4, i + 2, toYellow(str), false); 

                if (demo_summary.total_time)
                    Q_snprintfz(str, sizeof(str), "%15s", GetPrintedTime(demo_summary.total_time));
                else
                    Q_snprintfz(str, sizeof(str), "%15s", "N/A");

                qcurses_print(time_box, 0, i + 2, str, true); 
            }
        }
    }

    int filled_cols = 0;
    qcurses_insert(local_box, filled_cols, 0, name_box);
    qcurses_insert(local_box, filled_cols = filled_cols + name_box->cols, 0, map_box);
    qcurses_insert(local_box, filled_cols = filled_cols + map_box->cols, 0, player_box);
    qcurses_insert(local_box, filled_cols = filled_cols + player_box->cols, 0, skill_box);
    qcurses_insert(local_box, filled_cols = filled_cols + skill_box->cols, 0, kill_box);
    qcurses_insert(local_box, filled_cols = filled_cols + kill_box->cols, 0, secret_box);
    qcurses_insert(local_box, filled_cols = filled_cols + secret_box->cols, 0, time_box);
    qcurses_insert(local_box, filled_cols = filled_cols + time_box->cols, 0, size_box);
    qcurses_insert(main_box, start_col, start_row, local_box);

    qcurses_free(name_box);
    qcurses_free(map_box);
    qcurses_free(skill_box);
    qcurses_free(player_box);
    qcurses_free(kill_box);
    qcurses_free(secret_box);
    qcurses_free(time_box);
    qcurses_free(size_box);
    qcurses_free(local_box);
}

char * read_in_news(const char * filename){
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    return string;
}

qcurses_char_t * parse_news(char * html){
    qboolean tag = false;
    int color = 0;
    int i = 0, j = 0;
    char * src = Q_strcasestr(html, "<p class=\"d\">");
    char * end = Q_strcasestr(src + 1, "<p class=\"d\">");
    qcurses_char_t * qstr = calloc(strlen(html), sizeof(qcurses_char_t));
    for (i = 0; i < end - src; i++){
        if( src[i] == '<'){
            tag = true;
            if(!Q_strncasecmp(src + i, "<p class=\"d\"", 12) || !Q_strncasecmp(src + i, "<p class=\"s\"", 12) || !Q_strncasecmp(src + i, "<span class=\"y\">", 16)) {
                color = 128;
            } else if (!Q_strncasecmp(src + i, "<a href=\"/quake/mkt.pl?level", 28)) {
                color = 128;
            } else if (!Q_strncasecmp(src + i, "<li>", 4)) {
                qstr[j++].symbol = 6 + 128;
                qstr[j++].symbol = ' ';
            } else if (!Q_strncasecmp(src + i, "</p>", 4)) {
                color = 0;
                qstr[j++].symbol = '\n';
            } else if (!Q_strncasecmp(src + i, "</span>", 7) || !Q_strncasecmp(src + i, "</a>", 4)) {
                color = 0;
            }
        }
        if(!tag)
            qstr[j++].symbol = src[i] + color;
        if( src[i] == '>')
            tag = false;
    }
    qstr[j].symbol = 0;

    free(html);

    return qstr;
}

void M_Demos_DisplayNews (int cols, int rows, int start_col, int start_row) {
    qcurses_box_t * local_box = qcurses_init(cols, rows);

    if (!news)
        news = parse_news(read_in_news("news.html"));

    qcurses_boxprint_wrapped(local_box, news, 4096);
    qcurses_insert(main_box, start_col, start_row, local_box);

    qcurses_free(local_box);
}

void M_Demos_DisplayBrowser (int cols, int rows, int start_col, int start_row) {
    qcurses_box_t * local_box = qcurses_init(cols, rows);

    qcurses_insert(main_box, start_col, start_row, local_box);

    qcurses_free(local_box);
}

void M_Demos_Display (int width, int height)
{
    if (!main_box)
        main_box = qcurses_init(width / 8, height / 8);

    if (!demos_update)
        goto display;

    switch (demos_tab) {
        case TAB_LOCAL_DEMOS:
            M_Demos_DisplayLocal(main_box->cols - 6, main_box->rows - 8, 3, 5);
            break;
        case TAB_SDA_NEWS:
            M_Demos_DisplayNews(main_box->cols - 6, main_box->rows - 8, 3, 5);
            break;
        case TAB_SDA_DATABASE:
            M_Demos_DisplayBrowser(main_box->cols - 6, main_box->rows - 8, 3, 5);
            break;
    }

    qcurses_print(main_box, main_box->cols / 4 - 5, 2, "LOCAL DEMOS", demos_tab == TAB_LOCAL_DEMOS);
    qcurses_print(main_box, 2 * main_box->cols / 4 - 4, 2, "SDA NEWS", demos_tab == TAB_SDA_NEWS);
    qcurses_print(main_box, 3 * main_box->cols / 4 - 4, 2, "SDA DEMOS", demos_tab == TAB_SDA_DATABASE);

    qcurses_print(main_box, 3, 4, "\x1d", false);
    for (int i = 4; i < main_box->cols - 4; i++)
        qcurses_print(main_box, i, 4, "\x1e", false);
    qcurses_print(main_box, main_box->cols - 4, 4, "\x1f", false);

display:
    qcurses_display(main_box);
    demos_update = false;
}
