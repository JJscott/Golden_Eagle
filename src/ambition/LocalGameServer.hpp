#ifndef LOCALGAMESERVER_HEADER
#define LOCALGAMESERVER_HEADER

namespace ambition {
	class LocalGameServer : public GameServer {
	public:
		int get_game_version() const override {
			return 1; // TODO: Make this use the correct build number stuff
		}
	};
}

#endif