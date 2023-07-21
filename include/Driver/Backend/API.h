#pragma once
#include "APITypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {

            class BGS_API BGS_API_INTERFACE IAdapter
            {
            public:
                virtual ~IAdapter() = default;

                BGS_FORCEINLINE const AdapterInfo& GetInfo() const { return m_info; }

                BGS_FORCEINLINE AdapterHandle GetHandle() const { return m_handle; }

            protected:
                AdapterInfo   m_info;
                AdapterHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE IFactory
            {
            public:
                virtual ~IFactory() = default;

                virtual RESULT CreateDevice( const DeviceDesc& desc, IDevice** ppDevice ) = 0;
                virtual void   DestroyDevice( IDevice** ppDevice )                        = 0;

                BGS_FORCEINLINE const AdapterArray& GetAdapters() const { return m_adapters; }
                BGS_FORCEINLINE const FactoryDesc&  GetDesc() const { return m_desc; }
                BGS_FORCEINLINE const FactoryHandle GetHandle() const { return m_handle; }

            protected:
                FactoryDesc   m_desc;
                AdapterArray  m_adapters;
                FactoryHandle m_handle;
            };

            class BGS_API BGS_API_INTERFACE IDevice
            {
            public:
                virtual ~IDevice() = default;

                BGS_FORCEINLINE const DeviceDesc& GetDesc() const { return m_desc; }

            protected:
                DeviceDesc   m_desc;
                DeviceHandle m_handle;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS