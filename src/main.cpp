#include <coreinit/thread.h>
#include <coreinit/time.h>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>

#include <language/gettext.h>
#include <romfs-wiiu.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/time.h>
#include <coreinit/userconfig.h>

#include <cstring>

typedef enum
{
   Swkbd_LanguageType__Japanese  = 0,
   Swkbd_LanguageType__English   = 1,
   Swkbd_LanguageType__French    = 2,
   Swkbd_LanguageType__German    = 3,
   Swkbd_LanguageType__Italian   = 4,
   Swkbd_LanguageType__Spanish   = 5,
   Swkbd_LanguageType__Chinese1  = 6,
   Swkbd_LanguageType__Korean    = 7,
   Swkbd_LanguageType__Dutch     = 8,
   Swkbd_LanguageType__Potuguese = 9,
   Swkbd_LanguageType__Russian   = 10,
   Swkbd_LanguageType__Chinese2  = 11,
   Swkbd_LanguageType__Invalid   = 12
} Swkbd_LanguageType;

static Swkbd_LanguageType sysLang;

void loadLanguage(Swkbd_LanguageType language)
{
	switch(language)
	{
		case Swkbd_LanguageType__Japanese:
			gettextLoadLanguage("romfs:/japanese.lang");
         break;
		case Swkbd_LanguageType__English:
			gettextLoadLanguage("romfs:/english.lang");
         break;
		case Swkbd_LanguageType__French:
			gettextLoadLanguage("romfs:/french.lang");
         break;
		case Swkbd_LanguageType__German:
			gettextLoadLanguage("romfs:/german.lang");
         break;
		case Swkbd_LanguageType__Italian:
			gettextLoadLanguage("romfs:/italian.lang");
         break;
		case Swkbd_LanguageType__Spanish:
			gettextLoadLanguage("romfs:/spanish.lang");
         break;
		case Swkbd_LanguageType__Chinese1:
			gettextLoadLanguage("romfs:/chinese1.lang");
         break;
		case Swkbd_LanguageType__Korean:
			gettextLoadLanguage("romfs:/korean.lang");
         break;
		case Swkbd_LanguageType__Dutch:
			gettextLoadLanguage("romfs:/dutch.lang");
         break;
		case Swkbd_LanguageType__Potuguese:
			gettextLoadLanguage("romfs:/portuguese.lang");
         break;
		case Swkbd_LanguageType__Russian:
			gettextLoadLanguage("romfs:/russian.lang");
         break;
		case Swkbd_LanguageType__Chinese2:
			gettextLoadLanguage("romfs:/chinese2.lang");
         break;
		default:
			gettextLoadLanguage("romfs:/english.lang");
         break;
	}
}

void getSystemLanguage() {
   UCHandle handle = UCOpen();
   if(handle < 1)
   {
      sysLang = Swkbd_LanguageType__English;
   }

   UCSysConfig *settings = (UCSysConfig*)MEMAllocFromDefaultHeapEx(sizeof(UCSysConfig), 0x40);
   if(settings == NULL)
   {
      UCClose(handle);
      sysLang = Swkbd_LanguageType__English;
   }

   strcpy(settings->name, "cafe.language");
   settings->access = 0;
   settings->dataType = UC_DATATYPE_UNSIGNED_INT;
   settings->error = UC_ERROR_OK;
   settings->dataSize = sizeof(Swkbd_LanguageType);
   settings->data = &sysLang;

   UCError err = UCReadSysConfig(handle, 1, settings);
   UCClose(handle);
   MEMFreeToDefaultHeap(settings);
   if(err != UC_ERROR_OK)
   {
      sysLang = Swkbd_LanguageType__English;
   }
}

int main() {
   WHBProcInit();
   WHBLogConsoleInit();
   int res = romfsInit();
	if (res) {
		WHBLogPrintf(">> Failed to init romfs: %d", res);
		return 0;
	}
   
   getSystemLanguage();
   loadLanguage(sysLang);
   
   WHBLogPrintf(gettext("Hello World!"));

   while(WHBProcIsRunning()) {
      WHBLogConsoleDraw();
      OSSleepTicks(OSMillisecondsToTicks(100));
   }

   WHBLogPrintf("Exiting... good bye.");
   WHBLogConsoleDraw();
   OSSleepTicks(OSMillisecondsToTicks(1000));

   WHBLogConsoleFree();
   WHBProcShutdown();
   return 0;
}