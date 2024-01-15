#pragma once

namespace	common
{
	class	service_interface : private boost::noncopyable
	{
	protected:
		service_interface();

	public:
		int		run(int argc_, char** argv_);

	protected:
		virtual bool	on_create(int argc_, char** argv_) = 0;
		virtual void	on_destroy() = 0;

	private:
		static	void	callback_main();
		static	void	callback_handler(uint32_t control_);

		void			service_main();
		void			service_handler(uint32_t control_);
		void			set_status(uint32_t state_,
			uint32_t accept_ = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE);

	private:
		SERVICE_STATUS_HANDLE		_service_status_handle = nullptr;
		unsigned long				_state = 0;
		bool						_is_pause = false;
		HANDLE						_exit_event = nullptr;

		int							_argc = 0;
		char** _argv = nullptr;
		std::wstring				_service_name;

		static	service_interface* _this;
	};
}