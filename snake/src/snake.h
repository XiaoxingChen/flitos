#if !defined(__SNAKE_H__)
#define __SNAKE_H__

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <stdlib.h>
#include "ecs_vector.h"

void memFlip(std::array<uint16_t, 2>* p1, size_t length)
{
    auto p2 = p1 + length - 1;
    std::array<uint16_t, 2> cache;
    while(p1 < p2)
    {
        cache = *p2;
        *p2 = *p1;
        *p1 = cache;
        p1++;
        p2--;
    }
}

class Snake
{
public:

    enum GameState
    {
        eRUN = 1,
        eDEAD = 2,
        // eEAT = 3
    };

    Snake()
    {
        reset();
    }

    void unifyMemory()
    {
        if(0 == head_idx_) return;
        memFlip(mem_.data(), head_idx_);
        memFlip(mem_.data() + head_idx_, mem_.size() - head_idx_ - 1);
        memFlip(mem_.data(), mem_.size());
        head_idx_ = 0;
    }

    void reset()
    {
        for(size_t i = 0; i < mem_.size(); i++)
        {
            coord_to_erase_.push_back(mem_[i]);
            coord_to_erase_.push_back(food_coord_);
        }
        mem_.resize(1);
        mem_[0][0] = 5;
        mem_[0][1] = 5;
        game_state_ = Snake::eRUN;
    }

    void updatePhysics(const std::array<int16_t, 2>& cmd_dir)
    {
        if(game_state_ == Snake::eDEAD)
        {
            reset();
        }

        coord_to_erase_.clear();
        size_t tail_idx = (head_idx_ + mem_.size() - 1) % mem_.size();
        new_head_coord_ = mem_[head_idx_];
        if(0 == (cmd_dir[0] * curr_dir_[0] + cmd_dir[1] * curr_dir_[1]) && (cmd_dir[0] != 0 || cmd_dir[1] != 0))
        {
            curr_dir_ = cmd_dir;
            // std::cout << "dir: " << curr_dir_[0] << "," << curr_dir_[1] << std::endl;
        }

        new_head_coord_[0] += curr_dir_[0];
        new_head_coord_[1] += curr_dir_[1];
        // hit food
        if(new_head_coord_[0] == food_coord_[0] && new_head_coord_[1] == food_coord_[1] && mem_.size() < MAX_LENGTH)
        {
            unifyMemory();
            head_idx_ = mem_.size();
            mem_.push_back(new_head_coord_);
            updateFood();
            return;
        }
        // hit wall
        for(size_t i = 0; i < 2; i++)
        {
            if(bodyCoord(0)[i] == 0)
            {
                new_head_coord_[i] = (field_size_[i] + curr_dir_[i]) % field_size_[i];
            }
            if(new_head_coord_[i] == field_size_[i])
            {
                // game_state_ = Snake::eDEAD;
                // return;
                new_head_coord_[i] = field_size_[i] % new_head_coord_[i];
            }
        }
        // hit body
        for(size_t i = 0; i < mem_.size(); i++)
        {
            if(new_head_coord_[0] == mem_[i][0] &&  new_head_coord_[1] == mem_[i][1])
            {
                game_state_ = Snake::eDEAD;
                return;
            }
        }
        head_idx_ = (head_idx_ + 1) % mem_.size();
        coord_to_erase_.push_back({0,0});
        for(size_t i = 0; i < 2; i++)
        {
            coord_to_erase_.back()[i] = mem_[head_idx_][i];
            mem_[head_idx_][i] = new_head_coord_[i];
        }
    }

    void updateFood()
    {
        bool valid_food = false;
        while(!valid_food)
        {
            for(size_t i = 0; i < 2; i++)
            {
                food_coord_[i] = (1 + rand()) % (field_size_[i]-1);
            }
            valid_food = true;
            for(size_t i = 0; i < mem_.size(); i++)
            {
                if(food_coord_[0] == mem_[i][0] && food_coord_[0] == mem_[i][1])
                {
                    valid_food = false;
                    break;
                }
            }
        }
    }

    std::array<uint16_t, 2> headCoord()
    {
        return mem_[head_idx_];
    }

    ecs::vector<uint16_t> offsetsToErase() const
    {
        ecs::vector<uint16_t> ret;
        for(size_t i = 0; i < coord_to_erase_.size(); i++)
        {
            ret.push_back(coord_to_erase_.at(i)[0] * field_size_[1] + coord_to_erase_.at(i)[1]);
        }
        return ret;
    }

    uint16_t newHeadCoord() const
    { return new_head_coord_[0] * field_size_[1] + new_head_coord_[1]; }

    uint16_t foodCoord() const
    { return food_coord_[0] * field_size_[1] + food_coord_[1]; }

    void setFoodCoord(const std::array<uint16_t,2>& coord)
    {
        food_coord_ = coord;
    }

    uint16_t size() const { return mem_.size(); }

    const std::array<uint16_t,2>& bodyCoord(size_t i) const
    {
        return mem_[(head_idx_ + i) % mem_.size()];
    }

    const std::array<int16_t,2>& currDir() const
    {
        return curr_dir_;
    }

    uint16_t bodyCoord1D(size_t i) const
    {
        return mem_[(head_idx_ + i) % mem_.size()][0] * field_size_[1] + mem_[(head_idx_ + i) % mem_.size()][1];
    }

private:
    static const size_t MAX_LENGTH = 300;
    GameState game_state_ = Snake::eRUN;
    std::array<int16_t, 2> curr_dir_{1,0};
    size_t head_idx_ = 0;
    ecs::vector<std::array<uint16_t, 2>> mem_;
    std::array<uint16_t, 2> food_coord_{20,20};
    std::array<uint16_t, 2> field_size_{36,64};
    std::array<uint16_t, 2> new_head_coord_;

    ecs::vector<std::array<uint16_t, 2>> coord_to_erase_;

};


#endif // __SNAKE_H__
