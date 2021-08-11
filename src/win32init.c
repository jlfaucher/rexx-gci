/*
 *  Generic Call Interface for Rexx
 *  Copyright © 2003-2004, Florian Groﬂe-Coosmann
 *  Contributed by Jeff Glatt, changes by Christian Kulovits.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ----------------------------------------------------------------------------
 *
 * This file contains the code for initializing the WIN32 dynamic link
 * library so that it works with various Windows REXX interpreters.
 */

#include "gci.h"


/*
 * We redirect each call to the interpreter's API calls to one of the following
 * pointers. These will be set up further down in the DLL's initialization
 * routine to fit the currently running interpreter.
 */
RxAllocateMemory      GCI_AllocateMemory;
RxFreeMemory          GCI_FreeMemory;
RxDeregisterFunction  GCI_DeregisterFunction;
RxRegisterFunctionDll GCI_RegisterFunctionDll;
RxVariablePool        GCI_VariablePool;

/*
 * Not every Rexx interpreter has RexxFreeMemory(). This is a replacement.
 */
static APIRET APIENTRY dummyFree( PVOID mem )
{
   GlobalFree( mem );
   return 0;
}

/*
 * Not every Rexx interpreter has RexxAllocateMemory(). This is a replacement.
 */
static PVOID APIENTRY dummyAlloc( ULONG bytes )
{
   return GlobalAlloc( GMEM_FIXED, bytes );
}

/****************************** DLLMain() *****************************
 * Automatically called by Win32 when the DLL is loaded or unloaded.
 */

BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   static const char *Interpreters[] = {
      "Regina",
      "Rexxapi", /* keep this on position 1 in all cases */
      "Reginald"
   };
   HMODULE modApi = NULL;   /* standard DLL */
   HMODULE modRexx = NULL;  /* ORexx workaround */
   int i;

   (lpvReserved = lpvReserved);

   switch (fdwReason)
   {
      case DLL_PROCESS_ATTACH:
         /*
          * Find out which Rexx interpreter loaded us. We can use
          * GetModuleHandle. We must no use LoadLibrary(), this will
          * probably load another interpreter than the current one.
          *
          * ORexx needs two different DLLs. Strange and useless, but we
          * want to support it. We link the modRexx to modApi in all other
          * cases.
          */
         for ( i = 0; i < elements( Interpreters ); i++ )
         {
            if ( ( modApi = GetModuleHandle( Interpreters[i] ) ) != NULL )
            {
               if ( i == 1 )
                  modRexx = GetModuleHandle( "REXX" );
               else
                  modRexx = modApi;
               break;
            }
         }
         if ( modApi == NULL )
         {
            /*
             * The last chance is to find it in the current executable.
             */
            if ( ( modApi = GetModuleHandle( NULL ) ) == NULL )
               return FALSE;
         }

         /*
          * Load every function listened above.
          */

         GCI_RegisterFunctionDll = (RxRegisterFunctionDll)
                           GetProcAddress( modApi, "RexxRegisterFunctionDll" );
         GCI_DeregisterFunction = (RxDeregisterFunction)
                            GetProcAddress( modApi, "RexxDeregisterFunction" );
         GCI_VariablePool = (RxVariablePool)
                                 GetProcAddress( modRexx, "RexxVariablePool" );
         GCI_AllocateMemory = (RxAllocateMemory)
                                GetProcAddress( modApi, "RexxAllocateMemory" );
         GCI_FreeMemory = (RxFreeMemory)
                                    GetProcAddress( modApi, "RexxFreeMemory" );
         if ( ( GCI_RegisterFunctionDll == NULL ) ||
              ( GCI_DeregisterFunction == NULL ) ||
              ( GCI_VariablePool == NULL ) )
            return FALSE;

         /*
          * Be tolerante for errors with the allocation routines.
          */
         if ( GCI_AllocateMemory == NULL )
            GCI_AllocateMemory = dummyAlloc;
         if ( GCI_FreeMemory == NULL )
            GCI_FreeMemory = dummyFree;

         /*
          * We don't need to do anything for THREAD ATTACH, so we can
          * disable this support.
          */
         DisableThreadLibraryCalls( hinstDLL );
         break;

      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
         break;
   }

   return TRUE;
}
