#include "TransientBuffer.h"
#include "Core/RenderContext.h"

TransientBuffer::TransientBuffer(Device& device, 
                                VkBufferUsageFlags usageFlags,
                                VkDeviceSize size,
                                VkDeviceSize sizeInByte)
    : _device(device) {
    _buffer = std::make_unique<Buffer>(device, 
                                    size,
                                    usageFlags, 
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
    _freeList.push_back(SubAlloc(0, sizeInByte));
}

TransientBuffer::~TransientBuffer() {}

SubAlloc TransientBuffer::allocate(VkDeviceSize size, CommandBuffer& commandBuffer) {
    SubAlloc ret;

    // Find a free sub alloc
    bool found = false;
    auto iter = _freeList.begin();
    auto fit_iter = iter;

    for(; iter != _freeList.end(); ++iter) {
        if (iter->length >= size) {
            fit_iter = iter;
            found = true;
            break;
        }
    }

    // reallocate a larger buffer
    if (!found) {
        const auto old_buffer_size = _buffer->getSize();
        const auto buffer_size = std::max(old_buffer_size * 2, old_buffer_size);
        SubAlloc alloc(old_buffer_size, buffer_size - old_buffer_size);
        // Merge the new buffer with the previous one if they are contiguous
        if(!_freeList.empty() && _freeList.back().offset + _freeList.back().length == alloc.offset) {
            _freeList.back().length += alloc.length;
        } else {
            _freeList.push_back(alloc);
        }
        fit_iter = _freeList.end();
        fit_iter--;
        // Copy the old buffer data to the new larger buffer

        std::unique_ptr<Buffer> larger_buffer = Buffer::FromBuffer(_device, 
                                                                    commandBuffer, 
                                                                    *_buffer, 
                                                                    _buffer->getUsageFlags(), 
                                                                    0, 
                                                                    buffer_size);
        _buffer = std::move(larger_buffer);
    }

    VkDeviceSize left_size = fit_iter->length - size;
    ret.offset = fit_iter->offset;
    ret.length = size;
    if (left_size == 0) {
        _freeList.erase(fit_iter);
    } else {
        fit_iter->offset += size;
        fit_iter->length = left_size;
    }

    return ret;
}

void TransientBuffer::deallocate(SubAlloc const & alloc) {
    if(alloc.length == 0) {
        return;
    }
    _freeList.push_back(alloc);
    _freeList.sort([](const SubAlloc &a, const SubAlloc &b) { return a.offset < b.offset; });

    for (auto it = _freeList.begin(); it != _freeList.end(); ++it) {
        auto next = std::next(it);
        if (next != _freeList.end() && it->offset + it->length == next->offset) {
            it->length += next->length;
            _freeList.erase(next);
            --it; // Recheck the current iterator after merging
        }
    }
}



