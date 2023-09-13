#pragma once //ensure this header file only included once in a compilation unit
#define WIN32_LEAN_AND_MEAN //exclude some less commonly used or unnecessary part in the windows API
#include <Windows.h>
#include <TlHelp32.h>

#include <cstdint> //a part of the Windows API and provides functions and structures for working with processes and threads on the Windows operating system.

// Non-owning View: Unlike std::string, which owns its character data, std::string_view is a non-owning view. It holds a pointer to the character data along with a size, but it doesn't manage the memory itself.

// Immutable: std::string_view is immutable, meaning you can't modify the characters it points to. It's primarily used for read-only operations.

// Lightweight: Since it's just a view, the memory overhead is minimal compared to a full std::string. This makes it efficient for passing substrings around or using as function arguments.

// String Literal Compatibility: std::string_view can be constructed from string literals without any conversion, making it convenient to work with both string literals and actual std::string instances.

#include <string_view>
using namespace std;
class Memory
{
private:
  //uintptr_t
  //Guaranteed Fit: std::uintptr_t is designed to be large enough to hold the value of a pointer to any object in memory, regardless of the size of the memory or the platform. This ensures that you can safely cast a pointer to a std::uintptr_t and then back to the pointer type without losing information.

  // Pointer-Arithmetic Compatibility: Since it's an integer type, you can perform arithmetic operations on std::uintptr_t values, like addition or subtraction. This can be useful for certain low-level operations involving memory addresses.

  // Conversion: You can use a cast to convert a pointer to a std::uintptr_t value and vice versa. This is useful when you need to serialize pointers or store them in an integer format
	std::uintptr_t processId = 0;
	void* processHandle = nullptr;

public:
	// Constructor that finds the process id
	// and opens a handle
	Memory(const std::string_view processName) noexcept
	{
		::PROCESSENTRY32 entry = { }; //a global namespace with structure_PROCESSENTRY32: store information about a process when enumerating processes using the Windows Tool Help API.

		entry.dwSize = sizeof(::PROCESSENTRY32); //store the coorect size of the process

		const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		while (::Process32Next(snapShot, &entry))
		{
			if (!processName.compare(entry.szExeFile))
			{
				processId = entry.th32ProcessID;
				processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
				break;
			}
		}

		// Free handle
		if (snapShot)
			::CloseHandle(snapShot);
	}

	// Destructor that frees the opened handle
	~Memory()
	{
		if (processHandle)
			::CloseHandle(processHandle);
	}

	// Returns the base address of a module by name
	const std::uintptr_t GetModuleAddress(const std::string_view moduleName) const noexcept
	{
		::MODULEENTRY32 entry = { };
		entry.dwSize = sizeof(::MODULEENTRY32);
    //auto: infer the type of the variable based on the expression on the right-hand side of the assignment.
    //use snap shot to store a snapshot of the actual process
		const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
    //unintptr_t: It's an unsigned integer type that is guaranteed to be large enough to hold the value of a pointer
		std::uintptr_t result = 0;

		while (::Module32Next(snapShot, &entry))
		{ 
      //module is not the same as the stored on in the entry structure
			if (!moduleName.compare(entry.szModule))
			{
				result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
				break;
			}
		}

		if (snapShot)
			::CloseHandle(snapShot);

		return result;
	}

	// Read process memory
	template <typename T>
	constexpr const T Read(const std::uintptr_t& address) const noexcept
	{
		T value = { };
		::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
		return value;
	}

	// Write process memory
	template <typename T>
	constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
	{
		::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
	}
};