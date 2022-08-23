/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <language/gettext.h>
#include <fs/CFile.hpp>
#include <utils/StringTools.h>


char* strdup (const char* s)
{
  size_t slen = strlen(s) + 1;
  char* result = (char *) malloc(slen);
  if(result == NULL)
    return NULL;

  memcpy(result, s, slen);
  return result;
}

typedef struct _MSG {
    uint32_t id;
    char* msgstr;
    struct _MSG *next;
} MSG;
static MSG *baseMSG = NULL;

#define HASHMULTIPLIER 31 // or 37

// Hashing function from https://stackoverflow.com/a/2351171
static inline uint32_t hash_string(const char *str_param) {
    uint32_t hash = 0;

    while(*str_param != '\0')
        hash = HASHMULTIPLIER * hash + *p++;

    return hash;
}

/* Expand some escape sequences found in the argument string.  */
static char *
expand_escape(const char *str) {
    char *retval, *rp;
    const char *cp = str;

    retval = (char *) malloc(strlen(str) + 1);
    if (retval == NULL) return NULL;
    rp = retval;

    while (*cp != '\0' && *cp != '\\')
        *rp++ = *cp++;

    if (*cp == '\0') goto terminate;
    do {

        /* Here cp[0] == '\\'.  */
        switch (*++cp) {
        case '\"': /* " */
            *rp++ = '\"';
            ++cp;
            break;
        case 'a': /* alert */
            *rp++ = '\a';
            ++cp;
            break;
        case 'b': /* backspace */
            *rp++ = '\b';
            ++cp;
            break;
        case 'f': /* form feed */
            *rp++ = '\f';
            ++cp;
            break;
        case 'n': /* new line */
            *rp++ = '\n';
            ++cp;
            break;
        case 'r': /* carriage return */
            *rp++ = '\r';
            ++cp;
            break;
        case 't': /* horizontal tab */
            *rp++ = '\t';
            ++cp;
            break;
        case 'v': /* vertical tab */
            *rp++ = '\v';
            ++cp;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            int ch = *cp++ - '0';

            if (*cp >= '0' && *cp <= '7') {
                ch *= 8;
                ch += *cp++ - '0';

                if (*cp >= '0' && *cp <= '7') {
                    ch *= 8;
                    ch += *cp++ - '0';
                }
            }
            *rp = ch;
        }
        break;
        case '\\':
            ++cp;
        default:
            *rp = '\\';
            break;
        }

        while (*cp != '\0' && *cp != '\\')
            *rp++ = *cp++;

    } while (*cp != '\0');

    /* Terminate string.  */
terminate:
    *rp = '\0';
    return retval;
}

static inline MSG *findMSG(uint32_t id) {
    for (MSG *msg = baseMSG; msg; msg = msg->next)
        if (msg->id == id)
            return msg;

    return NULL;
}

static MSG *setMSG(const char *msgid, const char *msgstr) {
    if(!msgstr)
        return NULL;

    uint32_t id = hash_string(msgid);
    MSG *msg = findMSG(id);
    if (!msg) {
        msg = (MSG *) malloc(sizeof(MSG));
        msg->id = id;
        msg->msgstr = expand_escape(msgstr);
        msg->next = baseMSG;
        baseMSG = msg;
        return NULL;
    }

    if (msgstr) {
        if (msg->msgstr) free(msg->msgstr);
        //msg->msgstr = strdup(msgstr);
        msg->msgstr = expand_escape(msgstr);
    }
    return msg;
}

extern "C" void gettextCleanUp(void) {
    while (baseMSG) {
        MSG *nextMsg = baseMSG->next;
        free(baseMSG->msgstr);
        free(baseMSG);
        baseMSG = nextMsg;
    }
}

extern "C" BOOL gettextLoadLanguage(const char* langFile) {
    char *lastID = NULL;
    gettextCleanUp();

    CFile file(langFile, CFile::ReadOnly);
    if (!file.isOpen())
        return false;

    std::string strBuffer;
    strBuffer.resize(file.size());
    file.read((uint8_t *) &strBuffer[0], strBuffer.size());
    file.close();

    //! remove all windows crap signs
    size_t position;
    while(1) {
        position = strBuffer.find('\r');
        if(position == std::string::npos)
            break;

        strBuffer.erase(position, 1);
    }

    std::vector<std::string> lines = StringTools::stringSplit(strBuffer, "\n");


    if(lines.empty())
        return false;

    for(unsigned int i = 0; i < lines.size(); i++) {
        std::string & line = lines[i];
        // lines starting with # are comments
        if (line[0] == '#')
            continue;
        else if (strncmp(line.c_str(), "msgid \"", 7) == 0) {
            char *msgid, *end;
            if (lastID) {
                free(lastID);
                lastID = NULL;
            }
            msgid = &line[7];
            end = strrchr(msgid, '"');
            if (end && end - msgid > 1) {
                *end = 0;
                lastID = strdup(msgid);
            }
        } else if (strncmp(line.c_str(), "msgstr \"", 8) == 0) {
            char *msgstr, *end;

            if (lastID == NULL) continue;

            msgstr = &line[8];
            end = strrchr(msgstr, '"');
            if (end && end - msgstr > 1) {
                *end = 0;
                setMSG(lastID, msgstr);
            }
            free(lastID);
            lastID = NULL;
        }

    }
    return true;
}

extern "C" const char *gettext(const char *msgid) {
    if(!msgid)
        return NULL;

    MSG *msg = findMSG(hash_string(msgid));
    return msg ? msg->msgstr : msgid;
}

