﻿using CulverinEditor;
using CulverinEditor.Debug;

public class DaenerysCD_Left : CoolDown
{
    public int max_charges = 3;
    private int current_charges = 3;

    public GameObject charges_obj;

    void Start()
    {
        charges_obj = GetLinkedObject("charges_obj");
        charges_obj.GetComponent<CompText>().SetText(current_charges.ToString());
    }

    public override void Update()
    {
        if (current_charges < max_charges)
        {
            act_time += Time.deltaTime;
            if (act_time >= cd_time)
            {
                if (in_cd == true)
                {
                    in_cd = false;
                    button_cd = GetLinkedObject("daenerys_button_left_obj").GetComponent<CompButton>();
                    button_cd.Activate();
                }

                current_charges++;
                charges_obj.GetComponent<CompText>().SetText(current_charges.ToString());
                Debug.Log("[error] increase charges");

                if (current_charges < max_charges)
                {
                    act_time = 0.0f;
                }
            }
        }
    }

    public override void OnClick()
    {
        if (GetLinkedObject("daenerys_obj").GetComponent<DaenerysController>().GetState() == 0
            && GetLinkedObject("player_obj").GetComponent<CharactersManager>().changing == false)
        {
            if (in_cd == false)
            {
                if (GetLinkedObject("daenerys_obj").GetComponent<DaenerysController>().OnLeftClick() == true)
                {
                    ActivateAbility();
                }                
            }
        }
    }

    public override void ActivateAbility()
    {
        //this_obj.GetComponent
        current_charges--;
        charges_obj.GetComponent<CompText>().SetText(current_charges.ToString());

        act_time = 0.0f;
        
        if (current_charges == 0)
        {
            button_cd = GetLinkedObject("daenerys_button_left_obj").GetComponent<CompButton>();
            button_cd.Deactivate();
            in_cd = true;
        }
    }

    public int GetCurrentCharges()
    {
        return current_charges;
    }
}