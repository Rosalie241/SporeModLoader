/*
 * SporeModLoader - https://github.com/Rosalie241/SporeModLoader
 *  Copyright (C) 2022 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SPOREMODMANAGERHELPERS_MALLOCWRAPPER_HPP
#define SPOREMODMANAGERHELPERS_MALLOCWRAPPER_HPP

#include <iostream>
#include <cstdlib>

namespace SporeModManagerHelpers
{
	template <typename T>
	class MallocWrapper
	{
	private:
	    T* buffer = nullptr;
	    size_t bufferSize = 0;

	public:
	    MallocWrapper() {}
	    ~MallocWrapper()
	    {
	        if (this->buffer != nullptr)
	        {
	            std::free(this->buffer);
	            this->buffer = nullptr;
	        }
	    }

	    void AllocateOnce(size_t size)
	    {
	        if (this->buffer == nullptr)
	        {
	            this->buffer = reinterpret_cast<T*>(std::malloc(size));
	            if (this->buffer == nullptr)
	            {
	                std::cerr << "Error: failed to allocate memory!" << std::endl;
	                std::exit(1);
	            }
	            this->bufferSize = size;
	        }
	    }

	    T* data()
	    {
	        return this->buffer;
	    }

	    size_t size()
	    {
	    	return this->bufferSize;
	    }
	};
}

#endif // SPOREMODMANAGERHELPERS_MALLOCWRAPPER_HPP