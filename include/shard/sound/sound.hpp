#pragma once

#include <string>
#include <map>

#include <miniaudio.h>

namespace shard{
    class Sound{
        public:
            enum class Result{
                SUCCESS = 0,
                FAILED  = 1
            };

            Sound();
            ~Sound();

            uint32_t load(const std::string& filePath);

            bool isPlaying(uint32_t sound);
            Result setVolume(float volume);
            Result play(const std::string& filePath);
            Result play(uint32_t sound);
            Result start();
            Result stop();
        private:
            Result convertMaResultToResult(ma_result result){
                if(result == MA_SUCCESS) return Result::SUCCESS;
                else return Result::FAILED;
            }

            ma_engine soundEngine = {};
            std::map<uint32_t, ma_sound> sounds = {};
            uint32_t currentSoundName = 1;
    };
} // namespace shard
