﻿using CulverinEditor;
using CulverinEditor.Debug;
using System;

public class Boss_BT : BT
{
    public enum BOSS_STATE
    {
        BOSS_PHASE1,
        BOSS_PHASE2,
        BOSS_DEAD
    }

    public GameObject enemies_manager = null;
    public GameObject player = null;
    public GameObject mesh = null;

    public float total_hp = 100;
    protected float current_hp;
    public BOSS_STATE phase = BOSS_STATE.BOSS_PHASE1;

    public float max_anim_speed = 1.5f;
    public float min_anim_speed = 0.5f;

    //distance Attack
    public float distance_attack_cooldown = 1.0f;
    public float distance_attack_damage = 1.0f;

    //AOE Attack
    public float aoe_attack_cooldown = 1.0f;
    public float aoe_attack_damage = 1.0f;

    //Strong Attack
    public float strong_attack_cooldown = 1.0f;
    public float strong_attack_damage = 1.0f;

    //Triple Attack
    public float triple_attack_cooldown = 1.0f;
    public float triple_attack_damage = 1.0f;

    public float damaged_limit = 0.6f;
    protected float cooldown = 0.0f;

    System.Random rand_gen = null;

    public override void Start()
    {
        GetLinkedObject("enemies_manager").GetComponent<EnemiesManager>().AddBoss(gameObject);
        rand_gen = new System.Random();

        //Phase1Textures();
        current_action = GetComponent<Idle_Action>();
        GetComponent<CompAnimation>().PlayAnimation("Idle");

        current_hp = total_hp;

        base.Start();
    }

    public override void Update()
    {
        base.Update();
    }

    public override void MakeDecision()
    {
        if (next_action.action_type != Action.ACTION_TYPE.NO_ACTION)
        {
            current_action = next_action;
            current_action.ActionStart();
            next_action = null_action;
            return;
        }

        if (current_action.action_type != Action.ACTION_TYPE.IDLE_ACTION && current_action.action_type != Action.ACTION_TYPE.DIE_ACTION)
        {
            int distance_x = GetDistanceXToPlayer();
            int distance_y = GetDistanceYToPlayer();
            cooldown -= Time.deltaTime;

            bool in_cd = cooldown <= 0.0;

            if (in_cd == true)
            {
                switch (phase)
                {
                    case BOSS_STATE.BOSS_PHASE1:
                        if (distance_x == 2 && distance_y == 0 || distance_x == 0 && distance_y == 2)
                        {
                            int rand = rand_gen.Next(1, 10);

                            if (rand > 3)
                            {
                                //distance attack
                                Debug.Log("Distance Attack");
                                cooldown = distance_attack_cooldown;
                                return;
                            }
                            else
                            {
                                GetComponent<InfiniteChasePlayer_Action>().SetChaseRange(1);
                                GetComponent<InfiniteChasePlayer_Action>().ActionStart();
                                current_action = GetComponent<InfiniteChasePlayer_Action>();
                                return;
                            }
                        }
                        else if (distance_x == 1 && distance_y == 0 || distance_x == 0 && distance_y == 1)
                        {
                            int rand = rand_gen.Next(1, 10);

                            if (rand > 3)
                            {
                                //AOE attack
                                Debug.Log("AOE Attack");
                                cooldown = aoe_attack_cooldown;
                                return;
                            }
                            else
                            {
                                //distance attack
                                Debug.Log("Distance Attack");
                                cooldown = distance_attack_cooldown;
                                return;
                            }
                        }
                        else
                        {
                            GetComponent<InfiniteChasePlayer_Action>().ActionStart();
                            current_action = GetComponent<InfiniteChasePlayer_Action>();
                            return;
                        }

                    case BOSS_STATE.BOSS_PHASE2:

                        if (distance_x == 2 && distance_y == 0 || distance_x == 0 && distance_y == 2)
                        {
                            int rand = rand_gen.Next(1, 10);

                            if (rand > 3)
                            {
                                //triple attack
                                Debug.Log("Triple Attack");
                                cooldown = triple_attack_cooldown;
                                return;
                            }
                            else
                            {
                                GetComponent<InfiniteChasePlayer_Action>().SetChaseRange(1);
                                GetComponent<InfiniteChasePlayer_Action>().ActionStart();
                                current_action = GetComponent<InfiniteChasePlayer_Action>();
                                return;
                            }
                        }
                        else if (distance_x == 1 && distance_y == 0 || distance_x == 0 && distance_y == 1)
                        {
                            int rand = rand_gen.Next(1, 10);

                            if (rand > 3)
                            {
                                //Strong attack
                                Debug.Log("Strong Attack");
                                cooldown = strong_attack_cooldown;
                                return;
                            }
                            else
                            {
                                //triple attack
                                Debug.Log("Triple Attack");
                                cooldown = triple_attack_cooldown;
                                return;
                            }
                        }
                        else
                        {
                            GetComponent<InfiniteChasePlayer_Action>().ActionStart();
                            current_action = GetComponent<InfiniteChasePlayer_Action>();
                            return;
                        }
                }
            }
        }
    }

    public bool ApplyDamage(float damage)
    {
        InterruptAction();
        next_action = GetComponent<GetHit_Action>();
        current_hp -= damage;
        current_interpolation = current_hp / total_hp;

        if (current_hp <= 0)
        {
            state = AI_STATE.AI_DEAD;
            phase = BOSS_STATE.BOSS_DEAD;
            next_action = GetComponent<Die_Action>();
            current_action.Interupt();

            //todosforme
            GetLinkedObject("enemies_manager").GetComponent<EnemiesManager>().DeleteBoss();
        }
        else if (phase != BOSS_STATE.BOSS_PHASE2 && current_hp < total_hp * damaged_limit)
        {
            phase = BOSS_STATE.BOSS_PHASE2;
            //Phase2Textures();
        }

        return true;
    }

    public void Phase2Textures()
    {
        mesh.GetComponent<CompMaterial>().SetAlbedo("enemy1_Color_Hit.png");
        mesh.GetComponent<CompMaterial>().SetNormals("enemy1_Normal_Hit.png");
        mesh.GetComponent<CompMaterial>().SetAmbientOcclusion("enemy1_AO_Hit.png");
    }

    public void Phase1Textures()
    {
        mesh.GetComponent<CompMaterial>().SetAlbedo("enemy1_Color.png");
        mesh.GetComponent<CompMaterial>().SetNormals("enemy1_Normal.png");
        mesh.GetComponent<CompMaterial>().SetAmbientOcclusion("enemy1_AO.png");
    }

    public bool InCombat()
    {
        return !(current_action.action_type == Action.ACTION_TYPE.IDLE_ACTION);
    }

    public void SetAction(Action.ACTION_TYPE type)
    {
        switch (type)
        {
            case Action.ACTION_TYPE.GET_HIT_ACTION: next_action = GetComponent<GetHit_Action>(); break;
            case Action.ACTION_TYPE.ENGAGE_ACTION: next_action = GetComponent<Engage_Action>(); break;

            default: Debug.Log("[error] Unknown action"); break;
        }
    }

    public void Activate()
    {
        next_action = GetComponent<Engage_Action>();
    }

    public int GetDistanceXToPlayer()
    {
        GetLinkedObject("player_obj").GetComponent<MovementController>().GetPlayerPos(out int x, out int y);
        int distance_x = Mathf.Abs(x - GetComponent<Movement_Action>().GetCurrentTileX());
        return distance_x;
    }

    public int GetDistanceYToPlayer()
    {
        GetLinkedObject("player_obj").GetComponent<MovementController>().GetPlayerPos(out int x, out int y);
        int distance_y = Mathf.Abs(y - GetComponent<Movement_Action>().GetCurrentTileY());
        return distance_y;
    }
}