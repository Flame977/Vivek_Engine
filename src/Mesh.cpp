#include "Mesh.h"


void Mesh::Destroy()
{
    if (m_device == VK_NULL_HANDLE)
        return;

    if (m_indexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
    }

    if (m_vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
{
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        // Clean up existing resources
        this->~Mesh();

        m_device = other.m_device;
        m_vertexBuffer = other.m_vertexBuffer;
        m_vertexBufferMemory = other.m_vertexBufferMemory;
        m_indexBuffer = other.m_indexBuffer;
        m_indexBufferMemory = other.m_indexBufferMemory;
        m_indexCount = other.m_indexCount;

        // Invalidate source
        other.m_device = VK_NULL_HANDLE;
        other.m_vertexBuffer = VK_NULL_HANDLE;
        other.m_vertexBufferMemory = VK_NULL_HANDLE;
        other.m_indexBuffer = VK_NULL_HANDLE;
        other.m_indexBufferMemory = VK_NULL_HANDLE;
        other.m_indexCount = 0;
    }
    return *this;
}

void Mesh::Bind(VkCommandBuffer cmd) const
{
    VkBuffer buffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(
        cmd,
        m_indexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );
}

void Mesh::Draw(VkCommandBuffer cmd) const
{
    vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0);
}