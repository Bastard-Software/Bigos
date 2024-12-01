#pragma once

#include "Core/CoreTypes.h"

#include "Platform/Event/IEvent.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Event
        {
            class BGS_API WindowResizeEvent : public IEvent
            {
            public:
                WindowResizeEvent( uint32_t width, uint32_t height )
                    : m_width( width )
                    , m_height( height )
                {
                }

                uint32_t GetWidth() const { return m_width; }
                uint32_t GetHeight() const { return m_height; }

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::WINDOW_RESIZE; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "WindowResize"; };

            private:
                uint32_t m_width;
                uint32_t m_height;
            };

            class BGS_API WindowCloseEvent : public IEvent
            {
            public:
                WindowCloseEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::WINDOW_CLOSE; }
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "WindowClose"; };
            };

            class BGS_API WindowFocusEvent : public IEvent
            {
            public:
                WindowFocusEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::WINDOW_FOCUS; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "WindowFocus"; };
            };

            class BGS_API WindowLostFocusEvent : public IEvent
            {
            public:
                WindowLostFocusEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::WINDOW_LOST_FOCUS; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "WindowLostFocus"; };
            };

            class BGS_API AppTickEvent : public IEvent
            {
            public:
                AppTickEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::APP_TICK; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "AppTick"; };
            };

            class BGS_API AppUpdateEvent : public IEvent
            {
            public:
                AppUpdateEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::APP_UPDATE; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "AppUpdate"; };
            };

            class BGS_API AppRenderEvent : public IEvent
            {
            public:
                AppRenderEvent() = default;

                static EVENT_TYPE   GetStaticEventType() { return EventTypes::APP_RENDER; };
                virtual EVENT_TYPE  GetEventType() const override { return GetStaticEventType(); };
                virtual const char* GetName() const override { return "AppRender"; };
            };

        } // namespace Event
    }     // namespace Platform
} // namespace BIGOS