import _octokit from '@octokit/rest'
const {Octokit} = _octokit
import {getEnv} from './util.js'
import {promises as fs} from 'fs'
import path from 'path'

let octokit = null

async function apiCall(ns, method, args)
{
    if(!octokit)
    {
        octokit = new Octokit({
            auth: getEnv('ACCESS_TOKEN')
        })
    }

    try{
        const [owner, repo] = getEnv('GITHUB_REPOSITORY').split('/')
        args = {owner, repo, ...args}
        const res = await octokit[ns][method](args)
        return res.data
    }catch(e){
        console.error(`API ERROR: ${ns}.${method}(${JSON.stringify(args)}): ${e.message}`)
        return null
    }
}

export async function getReleaseByTag(tag) {
    return await apiCall('repos', 'getReleaseByTag', {
        tag
    })
}

export async function deleteRef(ref) {
    return await apiCall('git', 'deleteRef', {
        ref
    })
}

export async function deleteTag(tag) {
    return await deleteRef(`tags/${tag}`)
}

export async function deleteRelease(release_id) {
    return await apiCall('repos', 'deleteRelease', {
        release_id
    })
}

export async function createRelease(tag_name, {target_commitish = undefined, name = tag_name, body = '', draft = false, prerelease = false}) {
    if(!target_commitish)
        target_commitish = getEnv('GITHUB_SHA')

    return await apiCall('repos', 'createRelease', {
        tag_name,
        target_commitish,
        name,
        body,
        draft,
        prerelease
    })
}

export async function createPreReleaseDraft(tag_name, body, name = tag_name) {
    return await createRelease(tag_name, {
        body,
        name,
        prerelease: true,
        draft: true
    })
}

export async function updateRelease(release_id, {tag_name = undefined, target_commitish = undefined, name = undefined, body = undefined, draft = undefined, prerelease = undefined}) {
    return await apiCall('repos', 'updateRelease', {
        release_id,
        tag_name,
        target_commitish,
        name,
        body,
        draft,
        prerelease
    })
}

export async function publishRelease(release_id) {
    return await updateRelease(release_id, {draft: false})
}

export async function uploadReleaseAsset(release_id, filename, name = undefined) {
    let data = null
    try {
        data = fs.readFile(filename)
    } catch(e) {
        console.error(`FILE READ ERROR: ${filename}: ${e.message}`)
        return null
    }

    if(!name)
        name = path.basename(filename)

    return await apiCall('repos', 'uploadReleaseAsset', {
        release_id,
        data,
        name
    })
}

export async function uploadReleaseAssets(release_id, filenames, try_all = false) {
    const result = {
        allDone: true
    }

    if(Array.isArray(filenames))
    {
        result.assets = []

        for(const filename of filenames) {
            const asset = await uploadReleaseAsset(release_id, filename)
            if(!asset)
            {
                result.allDone = false
                if(!try_all)
                    break
            }
            result.assets.push(asset)
        }
    }
    else
    {
        result.assets = {}

        for(const [name, filename] of Object.entries(filenames)) {
            const asset = await uploadReleaseAsset(release_id, filename, name)
            if(!asset)
            {
                result.allDone = false
                if(!try_all)
                    break
            }
            result.assets[name] = asset
        }
    }

    return result
}

export async function recreatePreReleaseWithTag(tag, body, filenames) {
    const result = {}

    result.newDraft = await createPreReleaseDraft(tag, body)
    if(!result.newDraft)
        return result

    result.uploadResult = await uploadReleaseAssets(result.newDraft.id, filenames)
    if(!result.uploadResult)
        return result

    result.oldRelease = await getReleaseByTag(tag)
    if(result.oldRelease)
        await deleteRelease(result.oldRelease.id)
    await deleteTag(tag)

    result.publishedRelease = await publishRelease(result.newDraft.id)

    return result
}
