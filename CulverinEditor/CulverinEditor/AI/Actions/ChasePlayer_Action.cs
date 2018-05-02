﻿using CulverinEditor;
using CulverinEditor.Debug;
using CulverinEditor.Pathfinding;

public class ChasePlayer_Action : Action
{
    PerceptionEvent event_to_react;
    Movement_Action move;
    ACTION_RESULT move_return;
    public bool forgot_event = false;
    public float check_player_timer = 1.0f;
    public GameObject player;
    float timer = 0.0f;
    private bool blocking = false;
    Enemy_BT bt;
    PerceptionSightEnemy percep_sight;
    CompAnimation comp_anim;

    void Start()
    {
        blocking = false;
        move = GetComponent<Movement_Action>();
        player = GetLinkedObject("player");
    }

    public ChasePlayer_Action()
    {
        action_type = ACTION_TYPE.CHASE_ACTION;
    }

    public override bool ActionStart()
    {
        event_to_react.start_counting = false;

        bt = GetComponent<EnemySword_BT>();
        if (bt == null)
            bt = GetComponent<EnemyShield_BT>();
        if (bt == null)
            bt = GetComponent<EnemySpear_BT>();

        percep_sight = GetComponent<PerceptionSightEnemy>();

        comp_anim = GetComponent<CompAnimation>();

        move.GoToPlayer((uint)bt.range);
        interupt = false;
        bool ret = move.ActionStart();
        return ret;
    }

    public override bool ActionEnd()
    {
        interupt = false;
        move.SetInterupt(false);
        blocking = false;
        return true;
    }

    public override ACTION_RESULT ActionUpdate()
    {
        if (forgot_event == true || move.NextToPlayer() == true || interupt == true )
            move.Interupt();

        if (forgot_event == false)
        {
            timer += Time.deltaTime;

            if (timer >= check_player_timer && move.CenteredInTile())
            {
                if (player.GetComponent<CharactersManager>().GetCurrentCharacterName() == "Jaime")
                {
                    comp_anim.PlayAnimationNode("Chase");
                    SetBlocking(false);
                    move.SetBlocking(false);
                }
                else
                {
                    SetBlocking(true);
                    move.SetBlocking(true);
                }

                timer = 0.0f;

                if(bt == GetComponent<EnemySpear_BT>())
                {
                    move.GoToPlayer(1);
                }
                else
                {
                    move.GoToPlayer((uint)bt.range);
                }

            }

            if (percep_sight.player_seen == false)
                event_to_react.start_counting = true;
            else
                event_to_react.start_counting = false;
        }
        ///Make Move update
        move_return = move.ActionUpdate();

        if (move_return != ACTION_RESULT.AR_IN_PROGRESS)
            move.ActionEnd();

        return move_return;
    }

    public void SetEvent(PerceptionEvent e)
    {
        event_to_react = e;
    }

    public void SetInterupt(bool i)
    {
        interupt = i;
    }

    public void SetBlocking(bool value)
    {
        blocking = value;
    }

    public bool IsBlocking()
    {
        return blocking;
    }

}