#pragma once


#include "engine/array.h"
#include "engine/delegate_list.h"
#include "engine/path.h"
#include "engine/mt/sync.h"


namespace Lumix
{


class Material;
class Resource;
struct ResourceType;
class WorldEditor;
struct Action;
class FileSystemWatcher;
class Metadata;
class StudioApp;


class LUMIX_EDITOR_API AssetBrowser
{
public:
	struct IPlugin
	{
		virtual ~IPlugin() {}

		virtual bool onGUI(Resource* resource, ResourceType type) = 0;
		virtual ResourceType getResourceType(const char* ext) = 0;
		virtual void onResourceUnloaded(Resource* resource) = 0;
		virtual const char* getName() const = 0;
		virtual bool hasResourceManager(ResourceType type) const = 0;
		virtual bool acceptExtension(const char* ext, ResourceType type) const = 0;
	};

	typedef DelegateList<void(const Path&, const char*)> OnResourceChanged;

public:
	AssetBrowser(StudioApp& app);
	~AssetBrowser();
	void onGUI();
	void update();
	const Array<Path>& getResources(int type) const;
	int getTypeIndex(ResourceType type) const;
	void selectResource(const Path& resource, bool record_history);
	bool resourceInput(const char* label, const char* str_id, char* buf, int max_size, ResourceType type);
	void addPlugin(IPlugin& plugin);
	void openInExternalEditor(Resource* resource);
	void openInExternalEditor(const char* path);
	void enableUpdate(bool enable) { m_is_update_enabled = enable; }
	OnResourceChanged& resourceChanged() { return m_on_resource_changed; }
	bool resourceList(char* buf, int max_size, ResourceType type, float height);

public:
	bool m_is_opened;

private:
	void onFileChanged(const char* path);
	void findResources();
	void processDir(const char* path, int base_length);
	void addResource(const char* path, const char* filename);
	void onGUIResource();
	void unloadResource();
	void selectResource(Resource* resource, bool record_history);
	int getResourceTypeIndex(const char* ext);
	bool acceptExtension(const char* ext, ResourceType type);
	void onToolbar();
	void goBack();
	void goForward();
	void toggleAutoreload();
	bool isAutoreload() const { return m_autoreload_changed_resource; }

	ResourceType getResourceType(const char* path) const;

private:
	StudioApp& m_app;
	Metadata& m_metadata;
	Array<Path> m_changed_files;
	OnResourceChanged m_on_resource_changed;
	Array<Path> m_history;
	int m_history_index;
	Array<IPlugin*> m_plugins;
	MT::SpinMutex m_changed_files_mutex;
	Array<Array<Path> > m_resources;
	Resource* m_selected_resource;
	WorldEditor& m_editor;
	FileSystemWatcher* m_watchers[2];
	int m_current_type;
	char m_filter[128];
	char m_patch_base_path[MAX_PATH_LENGTH];
	Path m_wanted_resource;
	bool m_autoreload_changed_resource;
	bool m_is_focus_requested;
	bool m_activate;
	bool m_is_update_enabled;
	Action* m_auto_reload_action;
	Action* m_back_action;
	Action* m_forward_action;
	Action* m_refresh_action;
};


} // namespace Lumix