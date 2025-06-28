#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <string>
#include <thread>
#include <chrono>

// executes command and get stdout as string
std::string exec(const char* cmd) {
    std::array<char, 256> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "";

    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        result += buffer.data();
    }
    if (!result.empty() && result.back() == '\n') result.pop_back();

    return result;
}

int main() {
    std::string prev_track = "";
    std::cout << "ui_start" << std::endl;
    std::cout << "layout_horizontal" << std::endl;
 //   std::cout << "type=\"button\", text=\"<\", name=\"back\"" << std::endl;
 //   std::cout << "type=\"button\", text=\"=\"" << std::endl;
 //   std::cout << "type=\"button\", text=\">\"" << std::endl;
    std::cout << "ui_end" << std::endl;
    
    while (true) {
        // find currently playing media player via playerctl
        std::string players_str = exec("playerctl -l");
        if (players_str.empty()) {
          // no media player found (ie. spotify, bandcmap, etc.)
        } else {
            std::string playingPlayer;
            size_t start = 0, end;
            bool foundPlaying = false;

            while ((end = players_str.find('\n', start)) != std::string::npos) {
                std::string player = players_str.substr(start, end - start);
                start = end + 1;

                std::string statusCmd = "playerctl -p " + player + " status";
                std::string status = exec(statusCmd.c_str());
                if (status == "Playing") {
                    playingPlayer = player;
                    foundPlaying = true;
                    break;
                }
            }

        /*
          "name=\"Test Name\", when_clicked=")
            echo "resize_me";
            # Change the text property of the button named "Test Name"
            echo "name=\"Test Name\", text=\"Button A was clicked!\"";
            ;;
        */

            // check last line if no trailing newline
            if (!foundPlaying && start < players_str.size()) {
                std::string player = players_str.substr(start);
                std::string statusCmd = "playerctl -p " + player + " status";
                std::string status = exec(statusCmd.c_str());
                if (status == "Playing") {
                    playingPlayer = player;
                    foundPlaying = true;
                }
            }

            if (foundPlaying) {
                std::string metaCmd = "playerctl -p " + playingPlayer + " metadata --format '{{ artist }} - {{ title }}'";
                std::string track = exec(metaCmd.c_str());
                
                if (track.empty()) {
                    track = "Unknown";
                    std::cout << "resize_me" << std::endl;
                }
                // if new track
                if (!(track == prev_track && track == "Unknown")) {
                    std::cout << "set_text_as_icon \""+ track + "\"" << std::endl;
                    std::cout << "resize_me" << std::endl;
                    prev_track = track;
                }
            } else {
                // clear title & widget.
                std::cout << "set_text_as_icon \"\"" << std::endl;
                std::cout << "resize_me" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
