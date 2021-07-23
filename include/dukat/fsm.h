#pragma once

#include <functional>
#include <robin_hood.h>

namespace dukat
{
    template <typename T>
    class FSM
    {
	public:
		typedef std::function<void(const std::string&)> state_callback;

    private:
		static constexpr auto default_state = "_default_";
        // Controller state map
		robin_hood::unordered_map<std::string,T> states;
        // ID of current state
        std::string cur_state;
		// Global listener invoked after each state change.
		state_callback state_change_listener;
        bool in_transition;

    public:
        FSM(void) : cur_state(default_state), in_transition(false) { }
        ~FSM(void) { }

        void add_state(const T& state) { states.emplace(state.id, state); }
        void remove_state(const std::string& id) { if (states.count(id)) { states.erase(id); } };

		void set_state_change_listener(const state_callback& listener) { state_change_listener = listener; }

        // Sets the current state without triggering state exit.                
		void set_state(const std::string& id);
        T* get_state(void) { return (cur_state == default_state) ? nullptr : &states.at(cur_state); }
		bool in_state(const std::string& id) const { return id == cur_state; }
        bool in_state(const std::vector<std::string>& list) const { return std::find(list.begin(), list.end(), cur_state) != list.end(); }

        // Transitions to a new state.
        bool transition_to(const std::string& id);
    };

	template<typename T>
	void FSM<T>::set_state(const std::string& id)
	{
		if (states.count(id)) 
		{
			auto next_state = states.at(id);
			if (next_state.on_enter != nullptr)
				next_state.on_enter(next_state); // no previous state available in this case
			cur_state = id;
			if (state_change_listener != nullptr)
				state_change_listener(cur_state);
		}
	}

    template<typename T>
    bool FSM<T>::transition_to(const std::string& id)
    {
        // cannot transition out of or into invalid state
        if (!states.count(cur_state) || !states.count(id))
            return false;

        // check that next state is a valid transition from previous
        auto& prev_state = states.at(cur_state);
        if (!prev_state.transitions.count(id))
            return false;

        if (in_transition)
            return false; // already in transition, undefined behavior

        in_transition = true;

        auto& next_state = states.at(id);

        // call state exit / entrance handlers
        if (prev_state.on_exit != nullptr)
            prev_state.on_exit(next_state);
        if (next_state.on_enter != nullptr)
            next_state.on_enter(prev_state);

        cur_state = id;

		if (state_change_listener != nullptr)
			state_change_listener(cur_state);

        in_transition = false;

        return true;
    }
}