#include "PelicanPCH.h"
#include "FileDialog.h"

#ifdef PELICAN_WINDOWS
#include <Windows.h>
#include <ShObjIdl.h>
#endif

namespace Pelican
{
	// TODO: make a Path utility, which points to a file or folder which has both string and wstring for ease of use.
	// TODO: add functionality for extension filter, multiple files etc.
	bool IO::FileDialog::OpenFileDialog(std::wstring& filePath)
	{
#ifdef PELICAN_WINDOWS
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog* pFileOpen;

			// Create the FileOpenDialog
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
				IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr))
			{
				// Show the open dialog box
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box
				if (SUCCEEDED(hr))
				{
					IShellItem* pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						// Display the file name to the user
						if (SUCCEEDED(hr))
						{
							MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
							std::wstringstream ss;
							ss << pszFilePath;
							filePath = ss.str();
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}

		return true;
#endif
	}
}
