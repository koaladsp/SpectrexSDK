#pragma once

/// Internal functionality.

#define KDSP_IMPL(_x) public: void* m_impl = nullptr;

#define KDSP_IMPL_VIRTUAL_INTERFACE(_x) public: virtual void* v_impl() noexcept = 0; virtual void* v_impl() const noexcept = 0;
#define KDSP_IMPL_VIRTUAL(_x) public: void* v_impl() noexcept override; void* v_impl() const noexcept override;
