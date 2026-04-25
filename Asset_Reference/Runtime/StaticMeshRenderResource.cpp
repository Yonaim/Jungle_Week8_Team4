#include "Asset/Runtime/StaticMeshRenderResource.h"

namespace Asset
{
    std::shared_ptr<FStaticMeshRenderResource>
    FStaticMeshRenderResource::Create(const FObjCookedData& CookedData, RHI::FDynamicRHI& RHI)
    {
        if (!CookedData.IsValid())
        {
            return nullptr;
        }

        RHI::FBufferDesc VertexDesc;
        VertexDesc.ByteWidth = static_cast<uint32>(CookedData.VertexData.size());
        VertexDesc.Stride = CookedData.VertexStride;
        VertexDesc.BindFlags = RHI::EBufferBindFlags::VertexBuffer;
        VertexDesc.Usage = RHI::EBufferUsage::Default;
        VertexDesc.CPUAccessFlags = RHI::ECPUAccessFlags::None;

        std::shared_ptr<RHI::FRHIVertexBuffer> VertexBuffer =
            RHI.CreateVertexBuffer(VertexDesc, CookedData.VertexData.data());
        if (VertexBuffer == nullptr)
        {
            return nullptr;
        }

        RHI::FBufferDesc IndexDesc;
        IndexDesc.ByteWidth = static_cast<uint32>(CookedData.Indices.size() * sizeof(uint32));
        IndexDesc.Stride = sizeof(uint32);
        IndexDesc.BindFlags = RHI::EBufferBindFlags::IndexBuffer;
        IndexDesc.Usage = RHI::EBufferUsage::Default;
        IndexDesc.CPUAccessFlags = RHI::ECPUAccessFlags::None;

        std::shared_ptr<RHI::FRHIIndexBuffer> IndexBuffer =
            RHI.CreateIndexBuffer(IndexDesc, RHI::EIndexFormat::UInt32, CookedData.Indices.data());
        if (IndexBuffer == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<FStaticMeshRenderResource> Resource =
            std::make_shared<FStaticMeshRenderResource>();
        Resource->VertexBuffer = std::move(VertexBuffer);
        Resource->IndexBuffer = std::move(IndexBuffer);
        Resource->VertexCount = CookedData.VertexCount;
        Resource->IndexCount = static_cast<uint32>(CookedData.Indices.size());
        return Resource;
    }

    bool FStaticMeshRenderResource::IsValid() const
    {
        return VertexBuffer != nullptr && IndexBuffer != nullptr;
    }

    void FStaticMeshRenderResource::Reset()
    {
        VertexBuffer.reset();
        IndexBuffer.reset();
        VertexCount = 0;
        IndexCount = 0;
    }
} // namespace Asset
