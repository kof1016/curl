#pragma once
#include "../Utility/IState.h"
#include "../Utility/DataDefine.h"

namespace Logic 
{
	class DownloadDiffState final : public Utility::IState
	{
	public:
		typedef std::function<void()> OnDone;
		DownloadDiffState(DataDefine::FileListData::ShareContent contents);
		~DownloadDiffState();
		void Enter() override;
		void Leave() override;
		void Update() override;
		void OnDoneEvent(OnDone&& callback);
	private:
		DataDefine::FileListData::ShareContent _Contents;
		OnDone _OnDone;
	};

}